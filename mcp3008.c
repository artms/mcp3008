/*
 *  Microchip MCP3008 8 channel ADC module
 */

#include <linux/module.h>
#include <linux/version.h>
#include <linux/kmod.h>

#include <linux/spi/spi.h>
#include <linux/regmap.h>
#include <linux/iio/iio.h>
#include <linux/iio/driver.h>
#include <linux/regulator/consumer.h>

#define MCP3008 0
#define MCP3008_CHANNELS 8
#define MCP3008_RESOLUTION 10

struct mcp3008 {
	struct device *dev;
	struct mutex lock;

	struct spi_message message;
	struct spi_transfer transfer;
	struct regmap *regmap;
	struct regulator *regulator;
	// we need 3 byte buffer, first byte gets start bit, second gets actually command, third one is just a filler
	u8 tx_buf[3];
	u8 rx_buf[3];
};

const struct regmap_config mcp3008_regmap_config = {
	.reg_bits          = 8,
	.val_bits          = 16,
	.reg_stride        = 1,
	.disable_locking   = true,
	.can_sleep         = true,
};


static int mcp3008_spi_reg_read(void *context, unsigned int reg, unsigned int *val)
{
	struct mcp3008 *mcp = context;
	struct spi_device *spi = to_spi_device(mcp->dev);
	int err;

	mcp->tx_buf[1] = reg;
	mutex_lock(&mcp->lock);
	err = spi_sync(spi, &mcp->message);
	mutex_unlock(&mcp->lock);
	if (unlikely(err != 0)) {
		return err;
	}
	*val = ((mcp->rx_buf[1] & 0x03) << 8) | mcp->rx_buf[2];
	return 0;
}

static const struct regmap_bus mcp3008_spi_regmap = {
	.reg_read = mcp3008_spi_reg_read,
};

static unsigned int mcp3008_channel_to_tx_reg(struct iio_chan_spec const *chan)
{
	unsigned int reg = 0;
	reg |= (!chan->differential) << 7 | chan->address << 4;
	return reg;
};

static int mcp3008_read_raw(struct iio_dev *indio_dev, struct iio_chan_spec const *chan, int *val, int *val2, long mask)
{
	struct mcp3008 *mcp = iio_priv(indio_dev);
	int ret = -EINVAL;
	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		ret = regmap_read(mcp->regmap, mcp3008_channel_to_tx_reg(chan), val);
		if (likely(ret == 0)) {
			ret = IIO_VAL_INT;
		}
		break;
	case IIO_CHAN_INFO_SCALE:
		*val = regulator_get_voltage(mcp->regulator) / 1000;
		*val2 = MCP3008_RESOLUTION;
		ret = IIO_VAL_FRACTIONAL_LOG2;
		break;
	default:
		break;
	}

	return ret;
};

static const struct spi_device_id mcp3008_idtable[] = {
	{ "mcp3008", MCP3008 },
	{ /* END OF LIST */ },
};

// Device Tree matches
static const struct of_device_id mcp3008_of_match[] = {
	{ .compatible = "microchip,mcp3008" },
	{ /* END OF LIST */ },
};

static const struct iio_chan_spec mcp3008_channels[] = {
	{
		.type = IIO_VOLTAGE,
		.channel = 0,
		.indexed = 1,
		.address = 0,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_type = BIT(IIO_CHAN_INFO_SCALE),
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 1,
		.indexed = 1,
		.address = 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_type = BIT(IIO_CHAN_INFO_SCALE),
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 2,
		.indexed = 1,
		.address = 2,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_type = BIT(IIO_CHAN_INFO_SCALE),
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 3,
		.indexed = 1,
		.address = 3,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_type = BIT(IIO_CHAN_INFO_SCALE),
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 4,
		.indexed = 1,
		.address = 4,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_type = BIT(IIO_CHAN_INFO_SCALE),
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 5,
		.indexed = 1,
		.address = 5,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_type = BIT(IIO_CHAN_INFO_SCALE),
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 6,
		.indexed = 1,
		.address = 6,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_type = BIT(IIO_CHAN_INFO_SCALE),
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 7,
		.indexed = 1,
		.address = 7,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_type = BIT(IIO_CHAN_INFO_SCALE),
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 0,
		.channel2 = 1,
		.indexed = 1,
		.address = 0,
		.differential = 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_type = BIT(IIO_CHAN_INFO_SCALE),
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 1,
		.channel2 = 0,
		.indexed = 1,
		.address = 1,
		.differential = 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_type = BIT(IIO_CHAN_INFO_SCALE),
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 2,
		.channel2 = 3,
		.indexed = 1,
		.address = 2,
		.differential = 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_type = BIT(IIO_CHAN_INFO_SCALE),
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 3,
		.channel2 = 2,
		.indexed = 1,
		.address = 3,
		.differential = 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_type = BIT(IIO_CHAN_INFO_SCALE),
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 4,
		.channel2 = 5,
		.indexed = 1,
		.address = 4,
		.differential = 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_type = BIT(IIO_CHAN_INFO_SCALE),
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 5,
		.channel2 = 4,
		.indexed = 1,
		.address = 5,
		.differential = 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_type = BIT(IIO_CHAN_INFO_SCALE),
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 6,
		.channel2 = 7,
		.indexed = 1,
		.address = 6,
		.differential = 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_type = BIT(IIO_CHAN_INFO_SCALE),
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 7,
		.channel2 = 6,
		.indexed = 1,
		.address = 7,
		.differential = 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_type = BIT(IIO_CHAN_INFO_SCALE),
	},
};

static const struct iio_info mcp3008_iio_info = {
	.read_raw = mcp3008_read_raw,
};

static int mcp3008_spi_probe(struct spi_device *spi)
{
	struct device *dev = &spi->dev;
	struct mcp3008 *mcp;
	struct iio_dev *indio_dev;
	int err;
	dev_info(dev, "starting probing\n");

	indio_dev = devm_iio_device_alloc(&spi->dev, sizeof(struct mcp3008));
	if (indio_dev == NULL) {
		return -ENOMEM;
	}

	mcp = iio_priv(indio_dev);
	mutex_init(&mcp->lock);
	mcp->dev = dev;
	//set start bit
	mcp->tx_buf[0] = 1;
	mcp->transfer.tx_buf = mcp->tx_buf;
	mcp->transfer.rx_buf = mcp->rx_buf;
	// we're always transferring 3 bytes
	mcp->transfer.len = 3;
	// preparing message template
	spi_message_init_no_memset(&mcp->message);
	spi_message_add_tail(&mcp->transfer, &mcp->message);

	mcp->regmap = devm_regmap_init(dev, &mcp3008_spi_regmap, mcp, &mcp3008_regmap_config);
	if (IS_ERR(mcp->regmap)) {
		dev_err(dev, "unable to setup regmap\n");
		return PTR_ERR(mcp->regmap);
	}

	mcp->regulator = devm_regulator_get(dev, "vref");
	if (IS_ERR(mcp->regulator)) {
		dev_err(dev, "unable to get regulator");
		return PTR_ERR(mcp->regulator);
	}

	err = devm_regulator_get_enable(dev, "vref");
	if (err != 0) {
		return err;
	}

	indio_dev->name = spi_get_device_id(spi)->name;
	indio_dev->modes = INDIO_DIRECT_MODE;
	indio_dev->channels = mcp3008_channels;
	indio_dev->num_channels = ARRAY_SIZE(mcp3008_channels);
	indio_dev->info = &mcp3008_iio_info;

	return devm_iio_device_register(dev, indio_dev);
}

static void mcp3008_spi_remove(struct spi_device *spi)
{
	dev_info(&spi->dev, "removing\n");
}

static struct spi_driver mcp3008_spi_driver = {
	.driver       = {
		.name = "mcp3008",
		.of_match_table = mcp3008_of_match,
	},
	.probe        = mcp3008_spi_probe,
	.remove       = mcp3008_spi_remove,
	.id_table     = mcp3008_idtable,
};

static int __init mcp3008_spi_init(void)
{
	pr_info("module mcp3008 init\n");
	return spi_register_driver(&mcp3008_spi_driver);
}
module_init(mcp3008_spi_init);

static void __exit mcp3008_spi_exit(void)
{
	pr_info("module mcp3008 exit\n");
	spi_unregister_driver(&mcp3008_spi_driver);
}
module_exit(mcp3008_spi_exit);

MODULE_AUTHOR("Arturas Moskvinas <arturas.moskvinas@gmail.com>");
MODULE_DESCRIPTION("Microchip MCP3008 8 channel ADC module");
MODULE_LICENSE("GPL v2");
