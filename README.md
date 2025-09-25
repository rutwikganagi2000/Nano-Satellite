# 🛰️ Nanosatellite Communication Sub-System 📡

A low-power transceiver design for CubeSat communications using commercially available components in the UHF band (433MHz). 🚀✨

## 🌟 Project Overview

This project addresses the critical challenge that communication subsystems contribute to at least 29% of CubeSat failures in the first 90 days on-orbit. 📊💥 By utilizing inexpensive, widely available commercial components, this design aims to improve reliability while reducing costs for nanosatellite missions. 💰🔧


## ⚙️ Technical Specifications

### 🎯 Key Features
- **Operating Frequency:** 📻 433 MHz (UHF band)
- **Power:** 🔋 Low power design for extended satellite operation
- **Modulation:** 📶 LoRa and FSK modulation support
- **Components:** 🛍️ Commercial off-the-shelf (COTS) parts
- **Standards:** 📏 PC104 compliant layout option
- **Error Correction:** 🛡️ BCH encoding implementation

### 🔧 Hardware Components
- **Transceiver:** 📱 SX1278 (Semtech) - supports both LoRa and FSK modulation
- **Power Management:** ⚡ Multi-stage voltage regulation (5V → 4V, 3.3V)
- **RF Amplification:** 🔊 RFFM6406 power amplifier
- **Layout:** 🖼️ 4-layer PCB design with proper RF shielding

## 🎨 Design Highlights

### 📡 Modulation Schemes
1. **🌊 LoRa Modulation**
   - 🛡️ High immunity to Doppler effect
   - 🚀 Suitable for satellite communications above 550km altitude
   - 🎵 Frequency modulated chirp-based transmission

2. **⚡ FSK Modulation**
   - 🔄 Traditional frequency shift keying
   - 1️⃣0️⃣ Binary high/low frequency representation
   - 📶 Reliable for short-range communications

### 🖥️ PCB Design
- **🔧 Prototype 1:** Custom layout (83.5×58.5mm)
- **📐 Prototype 2:** PC104 standard compliant (95.89×90.17mm)
- **✨ Features:** Via stitching, RF shielding, proper impedance control

## 📊 Test Results

### 📈 Performance Metrics
- **📡 SNR Achieved:** 6.5 dB (no antenna, 10m range)
- **💾 Data Rate:** Up to 359.4 bps demonstrated
- **📶 RSSI:** -108 dBm to -94.5 dBm range
- **🎯 Frequency Error:** ±480.25 Hz

### 🛡️ BCH Encoding Results
- ✅ Successfully implemented error correction
- 🔄 Original data recovery demonstrated
- 📦 Fixed-length 64-bit codewords with 56 information bits

## 📅 Project Timeline

| 🗓️ Phase | ⏰ Duration | 📋 Deliverables |
|-----------|-------------|-----------------|
| 📚 Literature Review | Jan-Feb 2021 | Research foundation |
| 🔬 Methodology | March 2021 | Technical approach |
| 🧪 Short Range Testing | April-May 2021 | LoRa/FSK validation |
| ⚡ Schematic Design | June 2021 | Circuit design |
| 🖼️ PCB Layout | July-Aug 2021 | Two prototype designs |
| 🔨 Implementation | Sept 2021 | Fabrication & testing |

## 🏆 Key Achievements

- ✅ Successful ground testing with established communication link 📡
- ✅ Implementation of both LoRa and FSK modulation schemes 🌊⚡
- ✅ BCH error correction coding implementation 🛡️
- ✅ Two complete PCB prototypes (custom and PC104 standard) 🖼️🔧
- ✅ Cost-effective design using commercial components 💰
- ✅ Comprehensive SNR and link budget analysis 📊

## 🚀 Future Work

- 🔭 Long-range testing to determine operational limits
- 🌦️ Weather-related phenomena impact assessment
- 🔗 Integration testing with complete CubeSat systems
- 🌌 Orbital environment validation

## 📚 Technical Documentation

The complete technical documentation includes: 📋
- ⚡ Detailed circuit schematics
- 🖼️ PCB layout files (KiCAD format)  
- 📊 Bill of materials with supplier information
- 🧪 Test procedures and results
- 📈 Link budget calculations
- 🛡️ BCH encoding/decoding algorithms

## 🛠️ Dependencies

- 🖥️ KiCAD for PCB design
- 💻 Arduino IDE for microcontroller programming
- 📱 SX1278 library for LoRa/FSK operations
- 🛡️ Custom BCH encoding implementation


## 📜 License

This project is developed for educational and research purposes. 🎓 Please contact the authors for usage permissions. ⚖️

---

**🔥 Note:** This communication subsystem design represents a significant step toward more reliable and cost-effective CubeSat communications, addressing one of the primary failure modes in small satellite missions. 🛰️🎯