<p align="center">
  <img src="https://img.shields.io/badge/C%2B%2B-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white">
  <img src="https://img.shields.io/badge/Visual_Studio-5C2D91?style=for-the-badge&logo=visual%20studio&logoColor=white">
  <img src="https://img.shields.io/badge/Windows-0078D6?style=for-the-badge&logo=windows&logoColor=white">
  <img src="https://img.shields.io/badge/build-works-green?style=for-the-badge">
  <img src="https://img.shields.io/badge/code%20quality-depends-orange?style=for-the-badge">
  <img src="https://img.shields.io/badge/version-¯\\_(ツ)_/¯-blue?style=for-the-badge">
</p>

---

## ⚡ DragonBurn (but stealthier)

This is a **fork** of [ByteCorum/DragonBurn](https://github.com/ByteCorum/DragonBurn), just… UserMode.  
no more private sketchy ass private drivers that you have no idea wtf they do on your pc

Everything here was generated, tweaked, or outright ruined by an AI. I reserve the right to update or break whatever I feel like, whenever I feel like. No deadlines. No apologies.

If you spot bugs or have ideas for improvements—great, open an issue. I might care. I might forget. Such is life.

---

### 🔧 What's actually different (and why you'll care)

- **No kernel driver dependency**  
  - **Old way**: Requires kernel mode driver + mapper setup  
  - **New way**: Pure user-mode with handle hijacking  
  No more dealing with `DragonBurn-kmd` or kernel mappers. Less setup, less hassle.

- **Handle hijacking memory access**  
  - **Old way**: DeviceIoControl calls through kernel driver  
  - **New way**: NT API handle hijacking techniques  
  Different approach, same results.

- **All the same features**  
  - ✨ **Aimbot**: Still works if you want it to  
  - 🎯 **ESP**: Still shows enemies through walls  
  - 📊 **Radar**: Still trash  
  - 🔫 **RCS**: Still trash  
  - ⚡ **TriggerBot**: Still shoots for your dead brain

- **Safer startup**  
  No more "Looking for PRIVATE kernel Drivers" Safer for your pc and works with not hassle.

---

### 🚀 How to not mess this up

1. **Requirements**  
   - Visual Studio (the real one, not Code)  
   - Windows (obviously)  
   - A functioning brain (optional but recommended)

2. **Building**  
   - Open `DragonBurn.sln`  
   - Hit F5 or whatever  
   - Run as Admin because Windows is paranoid

3. **Usage**  
   - Start CS2  
   - Run this thing  
   - Don't be obvious about it

---

### 📌 Disclaimer

If you need a step-by-step guide for "Hello World," this isn't the place. Proceed only if you enjoy a little DIY mayhem and understand that cheating might get you banned.

---

### 🤝 Credit

Props to [ByteCorum](https://github.com/ByteCorum/DragonBurn) for the OG code. The rest? AI horrorshow meets hacker’s dream.

---

### 🐞 Bugs & Suggestions

Open an issue. I'll glance at it. Maybe fix it. Maybe press "Close" out of boredom.

Enjoy—or don't.  
¯\\_(ツ)_/¯
