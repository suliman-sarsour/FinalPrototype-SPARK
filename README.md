# Smart Water & Climate-Smart Systems – IoT Training Prototypes

## Overview

This repository contains source codes, configurations, and prototype implementations developed within the **SPARK for Innovation & Creativity** training program.

The focus is on building **low-cost, scalable IoT-based systems** for:

* Smart Water Management
* Climate-Smart Monitoring
* Data-driven decision support aligned with **SDG 6 (Clean Water)** and **SDG 13 (Climate Action)**

The projects are designed for **training, experimentation, and field-ready prototyping** using embedded systems.

---

## Objectives

* Monitor water systems using smart sensors
* Detect leaks, pressure drops, and abnormal consumption
* Collect and analyze climate and environmental data
* Visualize data through dashboards
* Enable decision-making using simple analytics and alerts
* Demonstrate renewable energy integration (solar-powered IoT)

---

## System Architecture

**Typical system flow:**

1. Sensors collect physical parameters (flow, pressure, level, temperature, humidity, soil moisture)
2. Microcontroller (ESP32 / Arduino) processes data
3. Data is transmitted via WiFi
4. Cloud platforms store and visualize data
5. Alerts and dashboards support monitoring and decision-making

---

## Hardware Components

* ESP32 / Arduino boards
* Flow Sensors
* Pressure Sensors
* Water Level Sensors
* DHT11 / DHT22 (Temperature & Humidity)
* Soil Moisture Sensors
* Breadboards, wiring, resistors
* Solar panel, battery, charge controller (conceptual / optional)

---

## Software & Platforms

* Arduino IDE
* ESP32 Core
* Blynk
* ThingSpeak
* Node-RED
* Serial Monitor / Debug tools

---

## Repository Structure

```
├── smart-water/
│   ├── flow_sensor/
│   ├── pressure_monitoring/
│   ├── leak_detection/
│   └── tank_level_controller/
│
├── climate-smart/
│   ├── temperature_humidity/
│   ├── soil_moisture/
│   └── climate_dashboard/
│
├── solar-power/
│   └── power_calculations/
│
├── dashboards/
│   ├── blynk/
│   ├── thingspeak/
│   └── node-red/
│
├── docs/
│   └── diagrams_and_notes/
│
└── README.md
```

---

## How to Use

1. Clone the repository

```bash
git clone https://github.com/your-username/your-repo-name.git
```

2. Open the desired project folder
3. Upload the code to your ESP32 / Arduino
4. Configure WiFi credentials and dashboard tokens
5. Monitor data and alerts via the selected platform

---

## Educational Use

This repository is intended for:

* Engineering training programs
* IoT workshops and bootcamps
* Academic demonstrations
* Early-stage prototypes for smart infrastructure

---

## Contribution

Contributions are welcome for:

* Code optimization
* Additional sensor integration
* Improved analytics
* Documentation and diagrams

Please fork the repository and submit a pull request.

---

## License

This project is provided for **educational and non-commercial use** unless stated otherwise.

---

## Authors & Trainers

* Eng. Wedad Aljabari
* Eng. Suliman Sarsour

SPARK for Innovation & Creativity

---

## Disclaimer

This repository contains prototype-level implementations intended for learning and demonstration purposes. Field deployment requires proper calibration, validation, and safety considerations.
