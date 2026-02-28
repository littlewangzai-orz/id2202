object PrimesApp {

  // Function to generate the first 'n' prime numbers
  def primes(n: Int): List[Int] = {
    def isPrime(num: Int): Boolean = {
      if (num < 2) false
      else !(2 until num).exists(num % _ == 0)
    }

    def findPrimes(count: Int, current: Int, acc: List[Int]): List[Int] = {
      if (count == n) acc
      else if (isPrime(current)) findPrimes(count + 1, current + 1, acc :+ current)
      else findPrimes(count, current + 1, acc)
    }

    findPrimes(0, 2, List())
  }

  // Function to format the list of primes as a comma-separated string
  def pretty(primeList: List[Int]): String = {
    primeList.mkString(",")
  }

  // Main function to handle command-line arguments and output
  def main(args: Array[String]): Unit = {
    if (args.length != 1) {
      System.err.println("Error: Exactly one argument is required.")
      System.exit(2)
    } else {
      try {
        val numPrimes = args(0).toInt
        if (numPrimes < 1 || numPrimes > 100) {
          System.err.println("Error: The argument must be a number between 1 and 100.")
          System.exit(1)
        } else {
          val primeNumbers = primes(numPrimes)
          println(pretty(primeNumbers))
          System.exit(0)
        }
      } catch {
        case _: NumberFormatException =>
          System.err.println("Error: The argument must be an integer.")
          System.exit(2)
      }
    }
  }
}
