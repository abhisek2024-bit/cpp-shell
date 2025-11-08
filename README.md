# C++ Shell Project

A custom Unix-style shell written in C++ that supports:

- Command execution
- Input/output redirection (`>`, `<`)
- Piping (`|`)
- Background jobs (`&`)
- Job control (`jobs`, `fg`, `bg`)

## 🔧 Features

- `ls > out.txt` — output redirection
- `cat < out.txt` — input redirection
- `ls | grep cpp` — piping
- `sleep 10 &` — background job
- `jobs` — list jobs
- `fg PID` — bring job to foreground
- `bg PID` — continue job in background

## 📁 Files

- `main.cpp` — entry point
- `shell.h` — class definition
- `shell.cpp` — shell logic

## 📸 Screenshots

Add screenshots of your shell running:
- `sleep 10 &`
- `jobs`
- `fg PID`
- `ls > out.txt`

## 📜 How to Compile

```bash
g++ main.cpp shell.cpp -o myshell
./myshell
