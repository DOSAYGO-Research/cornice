# Cornice

Cornice is a tool designed to evaluate the avalanche properties of prime numbers and generators, all with the ultimate goal of finding prime-generator pairs that yield exceptional cryptographic hashing properties. Primes that exhibit strong avalanche properties with one large generator tend to (but not always) maintain similar performance across a wide range of generators. This consistency makes such primes inherently more reliable for generating avalanche effects, and good candidates to be used as constants in hash algorithms. In contrast, primes that perform poorly with certain generators often display inconsistent behavior, making them less suitable for hashing applications. Therefore, primes with robust and uniform avalanche properties across multiple generators are considered superior for achieving quality hash functions.

## The Name

The name "Cornice" takes inspiration from the snow formations that accumulate along mountain ridges. These distinctive overhanging shapes can release sudden, powerful avalanches, mirroring the cascading bit-flips that define desirable "avalanche effects" in hash functions. Just as a cornice can unexpectedly shift large amounts of snow, a hash function with good avalanche properties can dramatically change output bits when even a single input bit is altered.

## Story

Cornice was developed to help identify primes with outstanding avalanche characteristics for use in quality hash functions. Initially, it played a key role in creating the [Rain hash functions](https://github.com/DOSAYGO-Research/rain) suite, including the Rainstorm and Rainbow variants. Its ability to thoroughly test and document avalanche behavior has made it an invaluable resource for developers and researchers working in hashing or even cryptography. The insight it provides empowers you to pick prime-generator pairs confidently for your hashing systems!

## Motivation

Avalanche properties lie at the very heart of quality hash functions. A hash function is said to have a "good" avalanche effect if flipping just one input bit leads to a significant, unpredictable number of output bits changing—often about half the bits of the output are expected to flip on average. For a 64-bit prime-based hash, that means roughly 32 output bits should change when a single input bit is altered!

To ensure these properties, Cornice tests **primitive roots** (often called **generators**) modulo $`P`$. Generators ensure that all possible non-zero residues modulo $`P`$ are produced, which closely mimics the uniform behavior needed in hashing scenarios. This full coverage of the residue set helps avoid short cycles and non-uniform distributions, resulting in a thoroughly tested prime that can reliably produce high-quality avalanche characteristics.

## How It Works

Cornice systematically evaluates candidate prime numbers ($`P`$) and generators ($`G`$) by testing how input bit flips propagate through a simulated hashing process. Here’s an overview:

- **Prime Generation:** Finds large-ish (approx. 64-bit) primes.
- **Generator Selection:** Identifies generators that fully span the multiplicative group modulo $`P`$, ensuring a rich and uniform distribution of residues.
- **Bit-Flip Simulation:** Evaluates how changing a single input bit affects the output, recording the number of output bits that flip.
- **Statistical Analysis:** Collects extensive data—zero-bit percentages, mean number of changed bits, standard deviation, and a histogram of results—providing a crystal-clear picture of each prime-generator pair’s avalanche quality.
- **Ranking and Reporting:** Sorts prime-generator pairs based on statistical metrics, shining a spotlight on the best candidates for your hashing needs.

## Mathematical Justification

### Basic Number Theory Concepts

Cornice’s approach is rooted in the properties of prime numbers and the structure of groups formed under modular arithmetic. Let’s briefly cover some foundational ideas:

1. **Primes and Multiplicative Groups:**
   For a prime number $`P`$, the set $`\{1, 2, \dots, P-1\}`$ forms a multiplicative group $`\mathbb{Z}_P^*`$ under modular multiplication:

   $$\mathbb{Z}_P^* = \{ x \mid 1 \leq x \leq P-1 \text{ and } \gcd(x,P)=1 \}.$$

   Since $`P`$ is prime, every element $`1 \leq x < P`$ (except $`P`$ itself) is coprime with $`P`$, so $`\mathbb{Z}_P^*`$ is a group of size $`P-1`$.

2. **Generators (Primitive Roots):**
   A generator $`G`$ of $`\mathbb{Z}_P^*`$ is an element that can produce every non-zero residue modulo $`P`$ when raised to successive powers:

   $$G^k \mod P, \quad k = 1, 2, \dots, P-1$$

   runs through the entire set $`\{1, 2, \dots, P-1\}`$. Such a $`G`$ ensures a full cycle and thus a highly uniform distribution of values.

3. **Existence of Generators:**
   For every prime $`P`$, the group $`\mathbb{Z}_P^*`$ is cyclic. This guarantees at least one generator exists. Finding such a generator is central to Cornice’s method, as it forms the foundation for thorough avalanche tests.

### Verifying a Generator

To confirm that $`G`$ is a generator, we must ensure it does not generate any smaller subgroup than the full $`\mathbb{Z}_P^*`$. This involves factoring $`P-1`$:

$$P - 1 = q_1^{e_1} q_2^{e_2} \dots q_m^{e_m},$$

where $`q_1, q_2, \dots, q_m`$ are distinct prime factors of $`P-1`$.

A valid generator $`G`$ must satisfy:

$$G^{\frac{P-1}{q_i}} \not\equiv 1 \pmod{P}$$

for every prime factor $`q_i`$ of $`P-1`$. If any such congruence is $`1`$, $`G`$ isn’t a full-cycle generator.

### Correlation to Avalanche Properties

Choosing $`G`$ carefully ensures a broad, pseudo-random distribution of values. This diversity helps simulate the unpredictable output changes expected from a good hash function, and the large number of non-degenerate samples a generator produce allows much data to evaluate for avalanche. It also seems that generators allow primes to show off their 'best side' when measuring for avalanche ability. So, by thoroughly checking and ranking these prime-generator pairs, Cornice guides you toward combinations that naturally produce strong avalanche effects. The result is a greater chance of achieving uniform, high-quality avalanche behavior in your hash functions—something that can help strengthen your overall security posture.

## Build Instructions

Cornice primarily targets Ubuntu/Debian systems, but you can adapt the build process to other environments with a bit of extra work. Don’t worry—just follow the steps and you’ll be up and running quickly!

### Prerequisites
- **Compiler:** A C++17-capable compiler (GCC or Clang is perfect).
- **Libraries:** OpenMP and standard C++ libraries.

### Building on Ubuntu/Debian
```bash
sudo apt update
sudo apt install build-essential libomp-dev
git clone https://github.com/DOSAYGO-Research/Cornice.git
cd Cornice/src
make
```

### Building on macOS

*I had trouble getting this to work. It's just easier to build in Linux, but if you figure it out, please open a PR with better instructions!*

```bash
brew install llvm libomp

# Set environment variables for Clang and OpenMP
export CC=/usr/local/opt/llvm/bin/clang
export CXX=/usr/local/opt/llvm/bin/clang++
export LDFLAGS="-L/usr/local/opt/llvm/lib -lomp"
export CPPFLAGS="-I/usr/local/opt/llvm/include"

clang++ -o cornice main.cpp -fopenmp -std=c++17 $LDFLAGS $CPPFLAGS
```

## Running Cornice

Once built, run:
```bash
./cornice
```

Cornice uses parallelization for much of the code, and outputs top-performing prime-generator pairs, giving you an immediate glimpse into the avalanche qualities you’re dealing with. This makes refining and choosing your hash parameters so much smoother!

## Reading the Results

The output provides rich data, including:

- **$`P`$:** The prime number tested.
- **$`G`$:** The generator found for this prime.
- **Zero Bits %:** The percentage of trials where no bits changed—ideally near 0.
- **Mean:** The average number of bits changed per single-bit input modification. Near 32 for a 64-bit output is a great sign!
- **Stddev:** How much variation there is in the bit changes. Lower means more consistent avalanche.
- **Histogram:** A breakdown of how often a certain number of bits changed, providing a fuller picture of distribution quality.

### Example Output

Cornice displays results in the form of simple histograms and summary statistics. Each bar in the histogram represents how many trials changed that many bits. Good avalanche distributions will cluster somewhere around half the bit length of the prime. In this case as the primes are 64 bits wide (or close to it), the best distributions will be peaking at 30 - 32 bits, or thereabouts.

```
P: 1458353492150186011, G: 1261772940743487803, Zero bits %: 0, Mean: 27.9098, Stddev: 4.98006
Histogram:
  2 bits: (1813)
  4 bits: (904)
  5 bits: (798)
  6 bits: (1762)
  7 bits: (477)
  9 bits: (1967)
 10 bits: (10785)
 11 bits: # (30651)
 12 bits: ## (55708)
 13 bits: ### (73434)
 14 bits: ### (76229)
 15 bits: ### (66082)
 16 bits: ## (52844)
 17 bits: ## (45233)
 18 bits: ## (47220)
 19 bits: ### (64747)
 20 bits: ##### (104297)
 21 bits: ######## (172152)
 22 bits: ############# (268045)
 23 bits: ################### (390313)
 24 bits: ########################## (526586)
 25 bits: ################################# (661833)
 26 bits: ####################################### (782539)
 27 bits: ########################################### (871491)
 28 bits: ############################################# (915989)
 29 bits: ############################################# (913329)
 30 bits: ########################################### (860405)
 31 bits: ###################################### (764425)
 32 bits: ################################ (643416)
 33 bits: ######################### (510398)
 34 bits: ################### (382451)
 35 bits: ############# (270487)
 36 bits: ######## (179480)
 37 bits: ##### (112300)
 38 bits: ### (66604)
 39 bits: # (36829)
 40 bits: (19242)
 41 bits: (9385)
 42 bits: (4332)
 43 bits: (1830)
 44 bits: (782)
 45 bits: (272)
 46 bits: (91)
 47 bits: (27)
 48 bits: (10)
 49 bits: (5)
 50 bits: (1)
```

In this example, the mean is slightly less than 32 and the standard deviation suggests some variation, but overall the distribution still leans toward a solid avalanche effect. Cornice’s thorough reporting helps you decide if this prime and generator set meets your quality standards.

The larger `num_samples`, the more likely you will find good quality $`P`$-$`G`$ pairs, and better primes.

## Contributions

We welcome them. Come on, come all.

## License

Cornice is distributed under the Apache License 2.0. Check out the LICENSE file for details. You’re free to experiment, adapt, and improve it for your hash building adventures!

---

For more information and updates, visit [Cornice on GitHub](https://github.com/DOSAYGO-Research/Cornice). Keep innovating, stay positive, and enjoy exploring the world of cryptography with Cornice at your side!
