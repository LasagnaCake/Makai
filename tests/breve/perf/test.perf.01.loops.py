import time

print("Doing 100 million loops...")

start = time.time()

print("Loop iteration #0")

for i in range(int(10e8)):
	if not int((i+1) % 10e7):
		print(f"Loop iteration #{i+1}")

total_time = (time.time() - start)

print("\nResult::{")
print(f"  total-time: {total_time}")
print("}")