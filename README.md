# irock 74M

I bought a irock 74m led keyboard, but got no way to control the led.

This is my solution.

I reverse engineer the usb packet in this google sheet:

https://docs.google.com/spreadsheets/d/1USv1qFaWV14PxGkvopRJYdi818f88eJyBioqb9oLgBw/edit?usp=sharing

this project current contains a `http server`, a `bot`, and an `imgui ui`.

## server 

the server current accept message like

```json
{
    "mode": 3,
    "brightness": 3,
    "speed": 2
}
```

## ui (imgui)

the ImGui GUI allows configuring and sending JSON configuration (`mode`, `brightness`, `speed`) to the HTTP server interactively.

```sh
./build/irock-ui --url http://localhost:9100
```

## bot

the bot listen to `/dev/input/event3`, if there key input, it changes mode. but other stuff is also possible

## build

```sh
mkdir build
cd build
cmake ..
make -j8
```

## TODO

- [ ] convert http server to gRPC
- [ ] support custom commands
