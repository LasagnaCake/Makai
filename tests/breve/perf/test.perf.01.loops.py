import time

print("Doing 100 million loops...")

start = time.time()

print("Loop iteration #0")

for i in range(int(10e8)):
	if not int(i % 10e7):
		print(f"Loop iteration #{i}")

total_time = (time.time() - start)

print("Result::{")
print(f"total-time: {total_time}")
print("}")