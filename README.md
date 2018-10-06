<head>
<meta name="google-site-verification" content="Itbj7Pl6rXPRDRqpVhgFbV1YXWEwhTPwvodLhGutGDw" />
</head>

# irm_ir_cmd

Implementation of irMagician command mimicking bto_ir_cmd

## Getting Started

This is software for Ubuntu.

### Prerequisites

install libusb and libjson-c for build. 

```
sudo apt-get install libusb-1.0-0 libusb-1.0-0-dev
sudo apt-get install libjson-c3 libjson-c-dev
```

## How to use

### Receive infrared remote control

When the reception of the infrared remote control succeeds, it is output to the standard output in JSON format.

```
saitou155@Endeavor-AT991E:~/irm_ir_cmd$ ./irm_ir_cmd -r
Capturing IR...
{ "format": "raw", "freq": 38, "data": [ 122, 1, 203, 50, 42, 4, 23, 4, 6, 6, 7, 4, 9, 4, 8, 4, 7, 4, 9, 5, 8, 4, 8, 5, 9, 4, 5, 7, 9, 4, 21, 4, 35, 4, 21, 4, 7, 5, 21, 4, 8, 5, 22, 4, 8, 5, 6, 5, 8, 5, 7, 5, 9, 4, 8, 4, 8, 5, 8, 4, 21, 5, 9, 4, 20, 4, 20, 5, 20, 5, 21, 5, 8, 4, 9, 4, 21, 5, 7, 5, 20, 5, 20, 5, 20, 5, 21, 5, 6, 6, 21, 5, 214, 52, 25, 6, 7, 5, 20, 6, 6, 6, 7, 5, 6, 6, 7, 6, 5, 7, 6, 6, 7, 5, 7, 5, 7, 5, 7, 5, 7, 5, 20, 5, 7, 5, 7, 5, 7, 5, 7, 5, 7, 5, 7, 5, 7, 5, 7, 5, 7, 5, 20, 6, 5, 7, 7, 5, 7, 6, 5, 7, 7, 6, 6, 6, 6, 6, 7, 5, 20, 6, 7, 5, 20, 5, 20, 6, 20, 6, 20, 6, 7, 5, 7, 6, 19, 6, 6, 6, 20, 5, 20, 6, 20, 6, 20, 6, 7, 5, 20, 5, 213, 52, 26, 6, 6, 6, 19, 6, 7, 5, 6, 6, 7, 6, 7, 5, 7, 6, 7, 5, 7, 6, 6, 5, 6, 6, 6, 6, 6, 6, 20, 6, 7, 5, 6, 6, 7, 5, 6, 6, 7, 5, 7, 5, 7, 5, 7, 5, 7, 5, 20, 6, 7, 5, 7, 6, 7, 5, 7, 5, 7, 5, 7, 6, 6, 6, 7, 5, 20, 6, 7, 6, 19, 6, 20, 6, 20, 6, 20, 6, 6, 5, 7, 5, 20, 6, 7, 5, 20, 6, 20, 6, 19, 6, 20, 6, 7, 5, 20, 6 ], "postscale": 100 }
```

To save the standard output JSON output to a file, do as follows.

```
./irm_ir_cmd -r | tee panasonic_tv_power.json
```

### Transmit infrared remote control

For infrared remote control transmission, put JSON as an argument.

```
saitou155@Endeavor-AT991E:~/irm_ir_cmd$ ./irm_ir_cmd -t '{ "format": "raw", "freq": 38, "data": [ 122, 1, 203, 50, 42, 4, 23, 4, 6, 6, 7, 4, 9, 4, 8, 4, 7, 4, 9, 5, 8, 4, 8, 5, 9, 4, 5, 7, 9, 4, 21, 4, 35, 4, 21, 4, 7, 5, 21, 4, 8, 5, 22, 4, 8, 5, 6, 5, 8, 5, 7, 5, 9, 4, 8, 4, 8, 5, 8, 4, 21, 5, 9, 4, 20, 4, 20, 5, 20, 5, 21, 5, 8, 4, 9, 4, 21, 5, 7, 5, 20, 5, 20, 5, 20, 5, 21, 5, 6, 6, 21, 5, 214, 52, 25, 6, 7, 5, 20, 6, 6, 6, 7, 5, 6, 6, 7, 6, 5, 7, 6, 6, 7, 5, 7, 5, 7, 5, 7, 5, 7, 5, 20, 5, 7, 5, 7, 5, 7, 5, 7, 5, 7, 5, 7, 5, 7, 5, 7, 5, 7, 5, 20, 6, 5, 7, 7, 5, 7, 6, 5, 7, 7, 6, 6, 6, 6, 6, 7, 5, 20, 6, 7, 5, 20, 5, 20, 6, 20, 6, 20, 6, 7, 5, 7, 6, 19, 6, 6, 6, 20, 5, 20, 6, 20, 6, 20, 6, 7, 5, 20, 5, 213, 52, 26, 6, 6, 6, 19, 6, 7, 5, 6, 6, 7, 6, 7, 5, 7, 6, 7, 5, 7, 6, 6, 5, 6, 6, 6, 6, 6, 6, 20, 6, 7, 5, 6, 6, 7, 5, 6, 6, 7, 5, 7, 5, 7, 5, 7, 5, 7, 5, 20, 6, 7, 5, 7, 6, 7, 5, 7, 5, 7, 5, 7, 6, 6, 6, 7, 5, 20, 6, 7, 6, 19, 6, 20, 6, 20, 6, 20, 6, 6, 5, 7, 5, 20, 6, 7, 5, 20, 6, 20, 6, 19, 6, 20, 6, 7, 5, 20, 6 ], "postscale": 100 }'
Transfer IR...
```

To output the JSON file with this command, use the shell function as follows.

```
./irm_ir_cmd -t "$(cat panasonic_tv_power.json)"
      or
./irm_ir_cmd -t "`cat panasonic_tv_power.json`"
```

## License

This project is Free software.



