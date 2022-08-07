#include <bcm2835.h>
#include <iostream>
#include <vector>
#include <numeric>

#define CONTINUOS_MODE 0x54

#define COMD_WRITE_CONFIG 0b00001000
#define DATA_ONE_SPS_MODE 0b01000000
#define DATA_ONE_SHOT_MODE 0b00100000
#define COMD_READ_VALUE 0b01010000

float decode(const char* buf) {
    // 13bit data so the first 3 bits of the recieved data have to be cut.
    // 0.0625C by 1 
    // the 3 LSB bits are not part of temperture

    size_t data = (buf[0] << 8 | buf[1]);

    return static_cast<float>((data >> 3) / 16);
}

int main(int argc, char const* argv[])
{
    if (!bcm2835_init())
        return -1;

    auto success = bcm2835_spi_begin();
    if (!success) std::cout << "init failed" << std::endl;
    //Set CS pins polarity to low
    bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);
    std::cout << "set cs0 to low" << std::endl;
    // bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS1, 0);

    //Set SPI clock speed
    //  BCM2835_SPI_CLOCK_DIVIDER_65536 = 0,       ///< 65536 = 262.144us = 3.814697260kHz (total H+L clock period) 
    //  BCM2835_SPI_CLOCK_DIVIDER_32768 = 32768,   ///< 32768 = 131.072us = 7.629394531kHz
    //  BCM2835_SPI_CLOCK_DIVIDER_16384 = 16384,   ///< 16384 = 65.536us = 15.25878906kHz
    //  BCM2835_SPI_CLOCK_DIVIDER_8192  = 8192,    ///< 8192 = 32.768us = 30/51757813kHz
    //  BCM2835_SPI_CLOCK_DIVIDER_4096  = 4096,    ///< 4096 = 16.384us = 61.03515625kHz
    //  BCM2835_SPI_CLOCK_DIVIDER_2048  = 2048,    ///< 2048 = 8.192us = 122.0703125kHz
    //  BCM2835_SPI_CLOCK_DIVIDER_1024  = 1024,    ///< 1024 = 4.096us = 244.140625kHz
    //  BCM2835_SPI_CLOCK_DIVIDER_512   = 512,     ///< 512 = 2.048us = 488.28125kHz
    //  BCM2835_SPI_CLOCK_DIVIDER_256   = 256,     ///< 256 = 1.024us = 976.5625MHz
    //  BCM2835_SPI_CLOCK_DIVIDER_128   = 128,     ///< 128 = 512ns = = 1.953125MHz
    //  BCM2835_SPI_CLOCK_DIVIDER_64    = 64,      ///< 64 = 256ns = 3.90625MHz
    //  BCM2835_SPI_CLOCK_DIVIDER_32    = 32,      ///< 32 = 128ns = 7.8125MHz
    //  BCM2835_SPI_CLOCK_DIVIDER_16    = 16,      ///< 16 = 64ns = 15.625MHz
    //  BCM2835_SPI_CLOCK_DIVIDER_8     = 8,       ///< 8 = 32ns = 31.25MHz
    //  BCM2835_SPI_CLOCK_DIVIDER_4     = 4,       ///< 4 = 16ns = 62.5MHz
    //  BCM2835_SPI_CLOCK_DIVIDER_2     = 2,       ///< 2 = 8ns = 125MHz, fastest you can get
    //  BCM2835_SPI_CLOCK_DIVIDER_1     = 1,       ///< 1 = 262.144us = 3.814697260kHz, same as 0/65536
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_256);

    //Set SPI data mode
    //  BCM2835_SPI_MODE0 = 0,  // CPOL = 0, CPHA = 0, Clock idle low, data is clocked in on rising edge, output data (change) on falling edge
    //  BCM2835_SPI_MODE1 = 1,  // CPOL = 0, CPHA = 1, Clock idle low, data is clocked in on falling edge, output data (change) on rising edge
    //  BCM2835_SPI_MODE2 = 2,  // CPOL = 1, CPHA = 0, Clock idle high, data is clocked in on falling edge, output data (change) on rising edge
    //  BCM2835_SPI_MODE3 = 3,  // CPOL = 1, CPHA = 1, Clock idle high, data is clocked in on rising, edge output data (change) on falling edge
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);

    //Set with CS pin to use for next transfers
    bcm2835_spi_chipSelect(BCM2835_SPI_CS0);

    // BCM2835_SPI_BIT_ORDER_LSBFIRST 	LSB First
    // BCM2835_SPI_BIT_ORDER_MSBFIRST 	MSB First
    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);

    // 7    : 0
    // 6-5  : R/W read -> 1, write -> 0
    // 5-3  : Register addr
    // 2    : Continuous mode
    // 1-0  : 0
    char cmd_continuous = 0x54; //0b01010100
    bcm2835_spi_transfer(cmd_continuous);

    std::vector<float> temps;

    for (size_t j = 0; j < 7; j++)
    {
        for (size_t i = 0; i < 8; i++)
        {
            // wair for conversion to finish
            bcm2835_delay(240);

            // send 2byte buffuers for converted value 
            char raw_temps[2] = { 0, 0 };
            bcm2835_spi_transfern(raw_temps, 2);

            // for debug
            // std::cout << "raw_value: " << std::hex << static_cast<int>(raw_temps[0]) << " " << static_cast<int>(raw_temps[1]) << std::endl;

            temps.push_back(decode(raw_temps));
        }

        // for (auto&& i : temps)
        // {
        //     std::cout << i << " ";
        // }
        // std::cout << std::endl;

        // debug average temps after measuring 7 times
        std::cout << "temps: " << std::oct << std::accumulate(std::begin(temps), std::end(temps), 0.0e0) / temps.size() << " C" << std::endl;

        // reset temps
        std::vector<float> empty;
        temps.swap(empty);
    }

    bcm2835_spi_end();
    bcm2835_close();

    return 0;
}
