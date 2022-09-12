#include "LED_Flash.h"

#include <string>

class led_flash
{
public:
	led_flash(); // constructor
	~led_flash();;

	int a{};
	int b{};

private:
	int c_ = b + a;
	

	
};

led_flash::led_flash()
= default;

led_flash::~led_flash()
= default;
