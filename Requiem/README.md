# Furina's Requiem

A simple program to help with multiplication problems.

## Compilation

> Note: furina is up to 10^1000, if I have time, I'll do 10^1e6

### Windows
Run the batch file:
```batch
requiem.bat
```

### Linux/Mac
Run the shell script:
```bash
bash requiem.sh
```

## Running the Program

1. The compiled program will be in the `bin` folder as "Se mettre sur son trente-et-un.exe"
2. Run the program and follow the on-screen instructions
3. Enter your answers when prompted

## Checking Your Answers

There are Python helper scripts in the `src` folder to:
- Generate random digits (`create_random_digit.py`)
- Verify multiplication results (`check_multiply.py`)

To check your answers:
```bash
python src/check_multiply.py
```

## Note
Make sure you have Python installed to use the checking scripts.