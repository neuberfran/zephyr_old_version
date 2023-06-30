/*
 * Copyright (C) 2018, NXP
 *
 * SPDX-License-Identifier: GPL-2.0+ and/or BSD-3-Clause
 */

#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/kfifo.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>

/* Processor A Transmit Register 0 (MUA_ATR0) offset */
#define MU_ATR0_OFFSET		0x0

/* Processor A Receive Register 0 (MUA_ARR0) offset */
#define MU_ARR0_OFFSET		0x10

/* Processor A Status Register (MUA_ASR) offset */
#define MU_ASR_OFFSET		0x20

/* Processor A Control Register (MUA_ACR) offset */
#define MU_ACR_OFFSET		0x24

/* Character device name */
#define DEVICE_NAME		"mu"

/* Class name */
#define CLASS_NAME		"mu"

/* Number of Messaging Unit registers for transmit (same number for receive) */
#define MU_REGISTERS		4

/* Device ID */
#define DEVICE_ID_IMX6SX_MU	"fsl,imx6sx-mu"

/* Character device's data */
struct mu_device_data {
	struct device *mu_device;
	struct mutex rx_mutex;
	struct mutex tx_mutex;
	struct kfifo rx_fifo;
	struct kfifo tx_fifo;
	wait_queue_head_t rx_fifo_not_empty;
	wait_queue_head_t tx_fifo_not_full;
};

static int mu_device_set_fifo_size(const char *val,
				   const struct kernel_param *kp);

static int mu_device_open(struct inode *inode, struct file *filp);

static int mu_device_close(struct inode *inode, struct file *filp);

static ssize_t mu_device_read_locked(struct file *filp, char __user *buffer,
				     size_t length, loff_t *offset);

static ssize_t mu_device_write_locked(struct file *filp,
				      const char __user *buffer, size_t length,
				      loff_t *offset);

static int mu_probe(struct platform_device *pdev);

static int mu_remove(struct platform_device *pdev);

/* Module parameter for receive and transfer buffer size. */
static uint fifo_size = 256;

static const struct kernel_param_ops fifo_size_param_ops = {
	.set = mu_device_set_fifo_size,
	.get = param_get_uint,
};

module_param_cb(fifo_size, &fifo_size_param_ops, &fifo_size, 0444);
MODULE_PARM_DESC(fifo_size,
"Size of receive and transmit buffers of each character device in bytes.");

/* Messaging Unit device driver file operations */
static const struct file_operations mu_file_operations = {
	.read = mu_device_read_locked,
	.write = mu_device_write_locked,
	.open = mu_device_open,
	.release = mu_device_close,
};

/* Messaging Unit device IDs */
static const struct of_device_id mu_device_ids[] = {
	{ .compatible = DEVICE_ID_IMX6SX_MU },
	{ }
};

/* Messaging Unit device driver */
static struct platform_driver mu_driver = {
	.driver = {
		.name = "imx-mu",
		.owner = THIS_MODULE,
		.of_match_table = mu_device_ids,
	},
	.probe = mu_probe,
	.remove = mu_remove,
};

/* Data for each character device */
static struct mu_device_data mu_device_datas[MU_REGISTERS];

/* Base address of Messaging Unit */
static void __iomem *mu_base;

/* Messaging Unit device class instance */
static struct class *mu_class;

/* Major number of the registered character device */
static int major_number = -1;

/* Mutex for guarding the access to Messaging Unit A control register */
static DEFINE_MUTEX(mua_cr_mutex);

/* Validates and sets fifo_size module parameter */
static int mu_device_set_fifo_size(const char *val,
				   const struct kernel_param *kp)
{
	int res;
	uint n = 0;

	res = kstrtouint(val, 10, &n);
	if (res != 0 || n < 4)
		return -EINVAL;

	return param_set_uint(val, kp);
}

/* Opening of Messaging Unit device */
static int mu_device_open(struct inode *inode, struct file *filp)
{
	pr_devel("open /dev/" DEVICE_NAME "%d, fd=0x%p\n",
		 MINOR(inode->i_rdev), filp);

	return 0;
}

/* Closing of Messaging Unit device */
static int mu_device_close(struct inode *inode, struct file *filp)
{
	pr_devel("close /dev/" DEVICE_NAME "%d, fd=0x%p\n",
		 MINOR(inode->i_rdev), filp);

	return 0;
}

/* Read from Messaging Unit device */
static ssize_t mu_device_read(struct file *filp, char __user *buffer,
			      size_t length, loff_t *offset)
{
	ssize_t read;
	int minor = MINOR(filp->f_inode->i_rdev);
	struct mu_device_data *data = &mu_device_datas[minor];

	pr_devel("read /dev/" DEVICE_NAME "%d, fd=0x%p, length=%u\n",
		 minor, filp, length);

	if (kfifo_is_empty(&data->rx_fifo)) {
		if (filp->f_flags & O_NONBLOCK)
			return -EAGAIN;

		if (wait_event_interruptible(data->rx_fifo_not_empty,
					     !kfifo_is_empty(&data->rx_fifo))) {
			read = -ERESTARTSYS;
		}
	}

	if (kfifo_to_user(&data->rx_fifo, buffer, length, &read))
		return -EFAULT;

	pr_devel("read /dev/" DEVICE_NAME "%d, fd=0x%p, read %d bytes\n",
		 minor, filp, read);

	if (kfifo_len(&data->rx_fifo) <= (fifo_size - 3)) {
		/*
		 * The function mu_isr can read up to 3 bytes from the
		 * Messaging Unit. Since there is enough free capacity
		 * in the receive fifo, enable the receive interrupt
		 * for this device again.
		 */
		mutex_lock(&mua_cr_mutex);
		writel_relaxed(readl_relaxed(mu_base + MU_ACR_OFFSET)
						| (0x08000000U >> minor),
			       mu_base + MU_ACR_OFFSET);
		mutex_unlock(&mua_cr_mutex);
	}

	return read;
}

/* Read from Messaging Unit device, guarded by mutex */
static ssize_t mu_device_read_locked(struct file *filp, char __user *buffer,
				     size_t length, loff_t *offset)
{
	ssize_t read;
	int minor = MINOR(filp->f_inode->i_rdev);
	struct mu_device_data *data = &mu_device_datas[minor];

	mutex_lock(&data->rx_mutex);
	read = mu_device_read(filp, buffer, length, offset);
	mutex_unlock(&data->rx_mutex);

	return read;
}

/* Write to Messaging Unit device */
static ssize_t mu_device_write(struct file *filp, const char __user *buffer,
			       size_t length, loff_t *offset)
{
	ssize_t written;
	int minor = MINOR(filp->f_inode->i_rdev);
	struct mu_device_data *data = &mu_device_datas[minor];

	pr_devel("write /dev/" DEVICE_NAME "%d, fd=0x%p, length=%u\n",
		 minor, filp, length);

	if (kfifo_is_full(&data->tx_fifo)) {
		if (filp->f_flags & O_NONBLOCK)
			return -EAGAIN;

		if (wait_event_interruptible(data->tx_fifo_not_full,
					     !kfifo_is_full(&data->tx_fifo))) {
			return -ERESTARTSYS;
		}
	}

	if (kfifo_from_user(&data->tx_fifo, buffer, length, &written))
		return -EFAULT;

	pr_devel("write /dev/" DEVICE_NAME "%d, fd=0x%p, written=%d\n",
		 minor, filp, written);

	if (!kfifo_is_empty(&data->tx_fifo)) {
		/*
		 * Since the transmit fifo is not empty and its content could be
		 * written to the Messaging Unit, enable the transmit interrupt
		 * for this device again.
		 */
		mutex_lock(&mua_cr_mutex);
		writel_relaxed(readl_relaxed(mu_base + MU_ACR_OFFSET)
						| (0x00800000U >> minor),
			       mu_base + MU_ACR_OFFSET);
		mutex_unlock(&mua_cr_mutex);
	}

	return written;
}

/* Write to Messaging Unit device, guarded by mutex */
static ssize_t mu_device_write_locked(struct file *filp,
				      const char __user *buffer, size_t length,
				      loff_t *offset)
{
	ssize_t written;
	int minor = MINOR(filp->f_inode->i_rdev);
	struct mu_device_data *data = &mu_device_datas[minor];

	mutex_lock(&data->tx_mutex);
	written = mu_device_write(filp, buffer, length, offset);
	mutex_unlock(&data->tx_mutex);

	return written;
}

/* Initializes data for /dev/mu{minor_number} */
static int mu_init_device_data(int minor_number)
{
	int res;
	struct mu_device_data *data = &mu_device_datas[minor_number];

	if (kfifo_alloc(&data->rx_fifo, fifo_size, GFP_KERNEL)) {
		res = -ENOMEM;
		goto rx_fifo_fail;
	}

	if (kfifo_alloc(&data->tx_fifo, fifo_size, GFP_KERNEL)) {
		res = -ENOMEM;
		goto tx_fifo_fail;
	}

	/* Adjust fifo_size parameter for the real size of fifo. */
	fifo_size = kfifo_size(&data->rx_fifo);

	mutex_init(&data->rx_mutex);
	mutex_init(&data->tx_mutex);
	init_waitqueue_head(&data->rx_fifo_not_empty);
	init_waitqueue_head(&data->tx_fifo_not_full);

	data->mu_device = device_create(mu_class, NULL,
					MKDEV(major_number, minor_number), NULL,
					DEVICE_NAME "%d", minor_number);
	if (IS_ERR(data->mu_device)) {
		res = PTR_ERR(data->mu_device);
		pr_err("failed to create device /dev/" DEVICE_NAME "%d\n",
		       minor_number);
		goto device_fail;
	}

	return 0;

device_fail:
	kfifo_free(&data->tx_fifo);
tx_fifo_fail:
	kfifo_free(&data->rx_fifo);
rx_fifo_fail:
	return res;
}

/* Destroys data for /dev/mu{minor_number} */
static void mu_destroy_device_data(int minor_number)
{
	struct mu_device_data *data = &mu_device_datas[minor_number];

	device_destroy(mu_class, MKDEV(major_number, minor_number));
	kfifo_free(&data->rx_fifo);
	kfifo_free(&data->tx_fifo);
	mutex_destroy(&data->rx_mutex);
	mutex_destroy(&data->tx_mutex);
}

/* Messaging Unit IRQ handler */
static irqreturn_t mu_isr(int irq, void *param)
{
	int minor;
	u32 irqs;
	u32 reg;
	u32 count;
	struct mu_device_data *data;

	/* Read messaging unit status register */
	irqs = readl_relaxed(mu_base + MU_ASR_OFFSET);

	for (minor = 0; minor < MU_REGISTERS; minor++) {

		data = &mu_device_datas[minor];

		if (irqs & (0x08000000U >> minor)) {
			/*
			 * There are data available for this device to read
			 * from the Messaging Unit B. The receive register
			 * contains 1 - 3 bytes of data and this count is
			 * stored in MSB. Read it and copy the data into
			 * the receive fifo.
			 */
			reg = readl_relaxed(mu_base + MU_ARR0_OFFSET
					    + (minor * 4));
			count = (reg & 0x03000000U) >> 24;
			pr_devel(
			      "%s /dev/" DEVICE_NAME "%d rx %u bytes: 0x%08x\n",
			      __func__, minor, count, reg);
			kfifo_in(&data->rx_fifo, (char *)&reg, count);

			if (kfifo_len(&data->rx_fifo) > (fifo_size - 3)) {
				/*
				 * Next time the ISR is invoked, we could read
				 * up to 3 bytes and it could happen that it
				 * would not fit into the receive fifo.
				 * So the receive interrupt is disabled for this
				 * device until there is more space in the fifo.
				 */
				writel_relaxed(readl_relaxed(mu_base
							     + MU_ACR_OFFSET)
					       & ~(0x08000000U >> minor),
					       mu_base + MU_ACR_OFFSET);
			}

			/* Notify blocked readers, if any */
			wake_up_interruptible(&data->rx_fifo_not_empty);
		}

		if (irqs & (0x00800000U >> minor)) {
			/*
			 * Transmit register is empty - data could be written
			 * there for the Messaging Unit B to read it, if there
			 * is something in the transmit fifo.
			 */
			if (!kfifo_is_empty(&data->tx_fifo)) {
				reg = 0x00000000U;
				count = kfifo_out(&data->tx_fifo, (char *)&reg,
						  3);
				reg |= (count << 24);
				pr_devel(
			      "%s /dev/" DEVICE_NAME "%d tx %u bytes: 0x%08x\n",
					 __func__, minor, count, reg);
				writel_relaxed(reg, mu_base + MU_ATR0_OFFSET
					       + (minor * 4));
			}

			if (kfifo_is_empty(&data->tx_fifo)) {
				/*
				 * Transmit fifo is empty, disable the transmit
				 * interrupt for this device until there are
				 * some data again.
				 */
				writel_relaxed(readl_relaxed(mu_base
							     + MU_ACR_OFFSET)
					       & ~(0x00800000U >> minor),
					       mu_base + MU_ACR_OFFSET);
			}

			/* Notify blocked writers, if any */
			wake_up_interruptible(&data->tx_fifo_not_full);
		}
	}

	return IRQ_HANDLED;
}

/* Messaging Unit module probe */
static int mu_probe(struct platform_device *pdev)
{
	int res;
	int minor_number;
	u32 irq;
	struct device_node *np;

	pr_devel("%s\n", __func__);

	/* Map memory of Messaging Unit A */
	np = of_find_compatible_node(NULL, NULL, DEVICE_ID_IMX6SX_MU);
	mu_base = of_iomap(np, 0);
	if (!mu_base) {
		res = -ENOMEM;
		goto iomap_fail;
	}

	/* Register character device */
	major_number = register_chrdev(0, DEVICE_NAME, &mu_file_operations);
	if (major_number < 0) {
		pr_err("failed to register device '%s': error %d\n",
		       DEVICE_NAME, major_number);

		res = major_number;
		goto chrdev_fail;
	}

	/* Create device class */
	mu_class = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(mu_class)) {
		pr_err("failed to register device class '%s'\n", CLASS_NAME);
		res = PTR_ERR(mu_class);
		goto class_fail;
	}

	/*
	 * There are MU_REGISTERS number of transmit and receive registers.
	 * One character device shall be created for each of them.
	 */
	for (minor_number = 0; minor_number < MU_REGISTERS; minor_number++) {
		res = mu_init_device_data(minor_number);
		if (res)
			goto device_data_fail;
	}

	/* Register ISR for Messaging Unit interrupt */
	irq = platform_get_irq(pdev, 0);
	res = request_irq(irq, mu_isr, IRQF_EARLY_RESUME, "imx-mu", NULL);
	if (res) {
		pr_err("%s: register interrupt %d failed, rc %d\n",
		       __func__, irq, res);
		goto irq_fail;
	}

	/*
	 * Set the RIE0-RIE3 of MUA_ACR - Messaging Unit A receive interrupts
	 * are enabled for each device and it starts to receive the data and to
	 * fill the receive fifos even when no device has been open for reading
	 * yet.
	 */
	writel_relaxed(readl_relaxed(mu_base + MU_ACR_OFFSET) | 0x0F000000U,
		       mu_base + MU_ACR_OFFSET);

	/*
	 * Clear the TIE0-TIE3 of MUA_ACR - Messaging Unit A transmit interrupts
	 * are disabled for each device until there is anything to send.
	 */
	writel_relaxed(readl_relaxed(mu_base + MU_ACR_OFFSET) & ~0x00F00000U,
		       mu_base + MU_ACR_OFFSET);

	return 0;

irq_fail:
	minor_number = MU_REGISTERS;
device_data_fail:
	for (minor_number -= 1; minor_number >= 0; minor_number--)
		mu_destroy_device_data(minor_number);

	class_destroy(mu_class);
class_fail:
	unregister_chrdev(major_number, DEVICE_NAME);
chrdev_fail:
	iounmap(mu_base);
iomap_fail:
	return res;
}

/* Messaging Unit module remove */
static int mu_remove(struct platform_device *pdev)
{
	u32 irq;
	int minor_number;

	pr_devel("%s\n", __func__);

	irq = platform_get_irq(pdev, 0);
	free_irq(irq, NULL);

	/* Disable the RIE0-RIE3 and TIE0-TIE3 of MUA_ACR */
	writel_relaxed(readl_relaxed(mu_base + MU_ACR_OFFSET) & ~0x0FF00000U,
		       mu_base + MU_ACR_OFFSET);

	for (minor_number = 0; minor_number < MU_REGISTERS; minor_number++)
		mu_destroy_device_data(minor_number);

	class_destroy(mu_class);
	unregister_chrdev(major_number, DEVICE_NAME);
	iounmap(mu_base);

	return 0;
}

module_platform_driver(mu_driver);

MODULE_AUTHOR("NXP Semiconductors");
MODULE_DESCRIPTION("i.MX Messaging Unit device driver");
MODULE_LICENSE("Dual BSD/GPL");
