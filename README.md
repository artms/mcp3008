# STOP!!!

>[!CAUTION]
>If you're looking for Microchip MCP3008 linux kernel module - use upstream [mcp320x](https://github.com/torvalds/linux/blob/master/drivers/iio/adc/mcp320x.c) module instead. 
>
>This module is strictly for learning purposes!
>
Device Tree section for MCP3008 module, make sure to put it into appropriate SPI section:
```
mcp3008: adc@0 {
	compatible = "microchip,mcp3008";
	vref_supply = <&vcc3v3_sys>;
	reg = <0x0>;
};
```
