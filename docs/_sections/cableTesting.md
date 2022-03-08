---
layout: post
title: Digital Video Commander Cable Use and Testing
---

__Step 1:__ Test the cable to make sure the cable works by establishing a connection between a deck and a computer using the diagram below. If your computer does not have a USB port, and adapter such as for USB-C will be necessary. (Click for full size).

<a href="{{ site.baseurl }}/images/DigitalVideoCommander_DVC-Setup_V2_USB-B.png"><img alt="USB Connection" src="{{ site.baseurl }}/images/DigitalVideoCommander_DVC-Setup_V2_USB-B.png"></a>

__Step 2:__ Install the [sony9pin CLI](https://github.com/hideakitai/Sony9PinRemote),  and run “sony9pin” in the terminal.

__Step 3:__ Identify the name of your port that is being used by the cable adapter from the list produced.

__Step 4:__ Make sure your deck is set to remote mode and test to see if your deck responds to the DVC cable by running test commands in the following format: `sony9pin [PORT NUMBER] [option]`.

For example, to issue a play command to a deck connected through port 10, you would use: `sony9pin 10 p`.

The full list of available commands is:
* `-h`: help
* `-v`: verbose mode

The full list of available options is:
* `-`: interactive (1-char command then enter)
* `e`: eject
* `f`: fast forward
* `p`: play
* `r`: rewind
* `s`: stop
* `0`: status
* `1`: type
* `2`: timer1
* `3`: timer2
* `4`: ltc_tc_ub
* `5`: vltc_tc_ub