print("Doing 100 million loops...")

local start = os.clock()

print("Loop iteration #0")

for i=1,10e8 do
	if (i % 10e7) == 0 then
		print("Loop iteration #" .. i)
	end
end

local total = (os.clock() - start)

print("\nResult::{")
print("  total-time: " .. total)
print("}")