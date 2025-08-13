from random import randint

print("This program creates 'lmao1.txt' and 'lmao2.txt' with a random N-digit number.")
N = int(input("Enter the number of digits: "))

with open("lmao1.txt", "w") as file:
    for _ in range(N):
        file.write(str(randint(0, 9)))

with open("lmao2.txt", "w") as file:
    for _ in range(N):
        file.write(str(randint(0, 9)))
