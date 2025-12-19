import json
import math
import random
import os

TOTAL_TRIANGLES = 250
OUTPUT_FILE = "triangles_fp.json"

counts = {
    "right": TOTAL_TRIANGLES * 20 // 100,
    "equilateral": TOTAL_TRIANGLES * 20 // 100,
    "isosceles": TOTAL_TRIANGLES * 20 // 100,
    "scalene": TOTAL_TRIANGLES * 40 // 100,
}

# Use an environment variable for the seed, default to 42
seed = int(os.getenv("TRIANGLE_GEN_SEED", 42))
random.seed(seed)

jsonSeed = random.randint(0, 2**31 - 1)

print(f"Using seed: {seed}")

# Skip generation if the file already exists
if os.path.exists(OUTPUT_FILE):
    print(f"{OUTPUT_FILE} already exists. Skipping generation.")
    exit(0)


def compute_angles(a, b, c):
    # Law of cosines, returns angles in degrees
    A = math.degrees(math.acos((b*b + c*c - a*a) / (2*b*c)))
    B = math.degrees(math.acos((a*a + c*c - b*b) / (2*a*c)))
    C = 180.0 - A - B
    return round(A, 6), round(B, 6), round(C, 6)


def triangle_dict(a, b, c):
    A, B, C = compute_angles(a, b, c)
    return {
        "sides": [round(a, 6), round(b, 6), round(c, 6)],
        "angles": [A, B, C],
    }


def generate_right(n):
    base_triples = [
        (3.0, 4.0, 5.0),
        (5.0, 12.0, 13.0),
        (8.0, 15.0, 17.0),
    ]
    result = []
    scale = 1.1
    while len(result) < n:
        for a, b, c in base_triples:
            if len(result) >= n:
                break
            result.append(triangle_dict(a * scale, b * scale, c * scale))
        scale += 0.37
    return result


def generate_equilateral(n):
    result = []
    side = 1.5
    for _ in range(n):
        result.append({
            "sides": [round(side, 6)] * 3,
            "angles": [60.0, 60.0, 60.0],
        })
        side += 0.73
    return result


def generate_isosceles(n):
    result = []
    base = 2.0
    while len(result) < n:
        equal = base + random.uniform(0.3, 1.5)
        base_len = base
        if 2 * equal > base_len:
            result.append(triangle_dict(equal, equal, base_len))
        base += 0.5
    return result


def generate_scalene(n):
    result = []
    attempts = 0
    max_attempts = n * 100  # Prevent infinite loops
    while len(result) < n and attempts < max_attempts:
        a = random.uniform(2.0, 8.0)
        b = random.uniform(2.0, 8.0)
        c = random.uniform(abs(a - b) + 0.1, a + b - 0.1)
        # Check that all sides are sufficiently different
        sides_rounded = [round(a, 3), round(b, 3), round(c, 3)]
        if len(set(sides_rounded)) == 3:
            result.append(triangle_dict(a, b, c))
        attempts += 1
    return result


triangles = {
    "seed": jsonSeed,
    "right": generate_right(counts["right"]),
    "equilateral": generate_equilateral(counts["equilateral"]),
    "isosceles": generate_isosceles(counts["isosceles"]),
    "scalene": generate_scalene(counts["scalene"]),
}

with open(OUTPUT_FILE, "w") as f:
    json.dump(triangles, f, indent=2)

print(f"Generated {TOTAL_TRIANGLES} floating-point triangles in {OUTPUT_FILE}")
