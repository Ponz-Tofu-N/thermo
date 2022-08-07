#include <bcm2835.h>
#include <iostream>
#include <vector>
#include <numeric>

#define CONTINUOS_MODE 0x54

#define COMD_WRITE_CONFIG 0b00001000
#define DATA_ONE_SPS_MODE 0b01000000
#define DATA_ONE_SHOT_MODE 0b00100000
#define COMD_READ_VALUE 0b01010000

float decode(char* buf) {
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

    char cmd[2] = { 0x8,0x20 };

    // bcm2835_spi_transfer(cmd[0]);
    // bcm2835_spi_transfer(cmd[1]);
    bcm2835_spi_transfern(cmd, 2);
    bcm2835_delay(240);
    char cmd_read[2] = { 0, COMD_READ_VALUE };
    // char buf_read[2] = { 1 };

    bcm2835_spi_transfern(cmd_read, 2);
    std::cout << "res_check_oneshot: " << std::hex << static_cast<int>(cmd_read[0]) << " " << static_cast<int>(cmd_read[1]) << std::endl;
    // std::cout << "res_check_oneshot: " << std::hex << static_cast<int>(buf_read[0]) << " " << static_cast<int>(buf_read[1]) << std::endl;
    // std::cout << "res_check_oneshot: " << std::hex << static_cast<int>(cmd[0]) << " " << static_cast<int>(cmd[1]) << std::endl;
    // std::cout << "res_check_oneshot: " << std::hex << static_cast<int>(bcm2835_spi_transfer(COMD_READ_VALUE)) << " " << static_cast<int>(cmd[1]) << std::endl;



    // ret_cmd_check[0] = bcm2835_spi_transfern(0);
    // ret_cmd_check[1] = bcm2835_spi_transfer(0);
    // bcm2835_spi_transfernb(cmd_check, ret_cmd_check, 2);
    // bcm2835_spi_transfer(cmd_check);
    // std::cout << "res_check: " << std::hex << static_cast<int>(bcm2835_spi_transfer(cmd_check)) << " " << static_cast<int>(bcm2835_spi_transfer(cmd_check)) << std::endl;

    // bcm2835_spi_transfer(CONTINUOS_MODE);

    // bcm2835_delay(60);
    // bcm2835_spi_transfer(COMD_READ_VALUE);
    // char cmd_check[2] = { 0 };
    // char ret_cmd_check[2] = { 0 };

    // bcm2835_spi_transfernb(cmd_check, ret_cmd_check, 2);
    // // bcm2835_spi_transfer(cmd_check);
    // std::cout << "res_check: " << std::hex << static_cast<int>(ret_cmd_check[0]) << " " << static_cast<int>(ret_cmd_check[1]) << std::endl;
    // std::cout << "res_check: " << std::hex << static_cast<int>(bcm2835_spi_transfer(ret_cmd_check)) << std::endl;
    // std::cout << "check: " << static_cast<int>(bcm2835_spi_transfer(cmd_check)) << std::endl;
    // std::cout << "check: " << static_cast<int>(bcm2835_spi_transfer(cmd_check)) << std::endl;
    // std::cout << "check: " << static_cast<int>(bcm2835_spi_transfer(cmd_check)) << std::endl;
    // std::cout << "check: " << static_cast<int>(bcm2835_spi_transfer(cmd_check)) << std::endl;
    // std::cout << "check: " << static_cast<int>(bcm2835_spi_transfer(cmd_check)) << std::endl;
    // std::cout << "check: " << static_cast<int>(bcm2835_spi_transfer(cmd_check)) << std::endl;
    // std::cout << "check: " << static_cast<int>(bcm2835_spi_transfer(cmd_check)) << std::endl;
    // std::cout << "check: " << static_cast<int>(bcm2835_spi_transfer(cmd_check)) << std::endl;
    // std::cout << "check: " << static_cast<int>(bcm2835_spi_transfer(cmd_check)) << std::endl;
    // std::cout << "check: " << static_cast<int>(bcm2835_spi_transfer(cmd_check)) << std::endl;
    bcm2835_delay(70);
    for (size_t i = 0; i < 15; i++)
    {
        // std::vector<float> temps;
        // while (temps.size() < 7)


        // ret_cmd_check[0] = bcm2835_spi_transfer(COMD_READ_VALUE);

        char cmd_check = 0;
        char ret_cmd_check[2] = { 0 };

        ret_cmd_check[0] = bcm2835_spi_transfer(0);
        ret_cmd_check[1] = bcm2835_spi_transfer(0);

        // bcm2835_spi_transfernb(cmd_check, ret_cmd_check, 2);
        // bcm2835_spi_transfer(cmd_check);
        // std::cout << "res_check: " << std::hex << static_cast<int>(bcm2835_spi_transfer(cmd_check)) << " " << static_cast<int>(bcm2835_spi_transfer(cmd_check)) << std::endl;
        std::cout << "res_check: " << std::hex << static_cast<int>(ret_cmd_check[0]) << " " << static_cast<int>(ret_cmd_check[1]) << std::endl;



        // std::cout << static_cast<int>(buf[0]) << " " << static_cast<int>(buf[1]) << " " << static_cast<int>(buf[2]) << std::endl;
        // std::cout << "read_data" << j << ": ";
        // for (auto i = std::begin(buf); i != std::end(buf); i++)
        // {
        //     std::cout << std::hex << std::uppercase << static_cast<int>(*i) << " ";
        // }

        // std::cout << std::endl;

        bcm2835_delay(1000);

        // temps.push_back(decode(buf + 1));

    // bcm2835_delay(500);
    // std::cout << "temperture" << i << ": " << std::reduce(std::begin(temps), std::end(temps), 0.0e0) / temps.size() << std::endl;


        std::cout << "temp: " << decode(ret_cmd_check) << std::endl;
    }

    bcm2835_spi_end();
    bcm2835_close();

    return 0;
}
