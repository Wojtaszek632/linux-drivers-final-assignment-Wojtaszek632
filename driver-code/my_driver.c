#include <linux/module.h>
#include <linux/init.h>
#include <linux/mutex.h>
#include <linux/err.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/spi/spi.h>
#include <linux/iio/iio.h>
#include <linux/iio/sysfs.h>


enum {
	MAX6682MUA,
	MAX6675,
	MAX31855,
	MAX31855K,
	MAX31855J,
	MAX31855N,
	MAX31855S,
	MAX31855T,
	MAX31855E,
	MAX31855R,
};

static const unsigned long max6682ScanMasks[] = {0x3, 0};

struct maxim_data {
	struct spi_device *spi;

	u8 buffer[16] ____cacheline_aligned;
	char tc_type;
};

static const struct iio_chan_spec max6682mua_channels[] = {
	{
		.type = IIO_TEMP,
		.info_mask_separate =
			BIT(IIO_CHAN_INFO_RAW),
		.scan_index = 0,
		.scan_type = {
			.sign = 's',
			.realbits = 10,
			.storagebits = 16,
			.shift = 3,
			.endianness = IIO_BE,
		},
	},
	IIO_CHAN_SOFT_TIMESTAMP(1),
};


static int my_read(struct maxim_data *data,
				   struct iio_chan_spec const *chan, int *val)
{
	pr_info("K Reading from the driver...\n");
	unsigned int storage_bytes = 2;
	unsigned int shift = chan->scan_type.shift;
	__be16 buf16;
	int ret;

	pr_info("K Reading SPI\n");
	ret = spi_read(data->spi, (void *)&buf16, storage_bytes);
	*val = be16_to_cpu(buf16);

	if (ret)
		return ret;


	*val = sign_extend32(*val >> shift, chan->scan_type.realbits - 1);

	pr_info("K Done read\n");
	return 0;
}


static int read_raw(struct iio_dev *indio_dev,
				       struct iio_chan_spec const *chan,
				       int *val, int *val2, long mask)
{
	struct maxim_data *data = iio_priv(indio_dev);
	int ret = -EINVAL;

	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		ret = iio_device_claim_direct_mode(indio_dev);
		if (ret)
			return ret;

		ret = my_read(data, chan, val);
		iio_device_release_direct_mode(indio_dev);

		if (!ret)
			return IIO_VAL_INT;

		break;
	}

	return ret;
}

static const struct iio_info maxim_sensor_info = {
	.read_raw = read_raw,
};

static int my_probe(struct spi_device *spi)
{
	pr_info("K Probing the driver...\n");
	const struct spi_device_id *id = spi_get_device_id(spi);
	struct iio_dev *indio_dev;
	struct maxim_data *data;
	
	pr_info("K allocate iio_dev...\n");
	indio_dev = devm_iio_device_alloc(&spi->dev, sizeof(*data));
	if (!indio_dev)
		return -ENOMEM;

	pr_info("K allocated!\n");
	indio_dev->info = &maxim_sensor_info;
	indio_dev->name = "MAX6682MUA";
	indio_dev->channels = max6682mua_channels;
	indio_dev->available_scan_masks = max6682ScanMasks;
	indio_dev->num_channels = 1;
	indio_dev->modes = INDIO_DIRECT_MODE;

	data = iio_priv(indio_dev);
	data->spi = spi;

	if (id->driver_data == MAX6682MUA)
		dev_warn(&spi->dev, "driver matched!\n");

	return devm_iio_device_register(&spi->dev, indio_dev);
}

static const struct spi_device_id my_id_table[] = {
	{"max6682mua", MAX6682MUA},
	{"max31855", MAX31855},
	{"max31855k", MAX31855K},
	{"max31855j", MAX31855J},
	{"max31855n", MAX31855N},
	{"max31855s", MAX31855S},
	{"max31855t", MAX31855T},
	{"max31855e", MAX31855E},
	{"max31855r", MAX31855R},
	{},
};
MODULE_DEVICE_TABLE(spi, my_id_table);




static const struct of_device_id my_match_table[] = {
    { .compatible = "maxim,max6682mua" },
    { },
};
MODULE_DEVICE_TABLE(of, my_match_table);




static struct spi_driver MAX_driver = {
	.driver = {
		.name	= "MAX6682MUA_driver",
		.of_match_table = my_match_table,
	},
	.probe		= my_probe,
	.id_table	= my_id_table,
};
module_spi_driver(MAX_driver);

MODULE_LICENSE("GPL");