#! /usr/bin/python3

from math_parser import *
import io
import unittest


def testFactor_init(self, f_value, test_value):

  factor = Factor(f_value)
  self.assertEqual(str(factor._value), test_value)
  self.assertEqual(factor._exp, 1)

  factor = Factor(f_value, 1)
  self.assertEqual(str(factor._value), test_value)
  self.assertEqual(factor._exp, 1)

  factor = Factor(f_value, Number(1))
  self.assertEqual(str(factor._value), test_value)
  self.assertEqual(factor._exp, 1)

  factor = Factor(f_value, 2)
  self.assertEqual(str(factor._value), test_value)
  self.assertEqual(factor._exp, 2)

  factor = Factor(f_value, Number(2))
  self.assertEqual(str(factor._value), test_value)
  self.assertEqual(factor._exp, 2)

  factor = Factor(f_value, -2)
  self.assertEqual(str(factor._value), test_value)
  self.assertEqual(factor._exp, -2)

  with self.assertRaises(TypeError):
    factor = Factor(f_value, "2")

def testParseFactorType(self, f_value, test_value, expected_type):

  with io.StringIO(f_value) as f:
    c = f.read(1)
    factor, c = parse_factor(c, f)
    self.assertEqual(str(factor), test_value)
    self.assertEqual(factor._exp, 1)
    self.assertIsInstance(factor, Factor)
    self.assertIsInstance(factor._value, expected_type)

  with io.StringIO(f_value + "^1") as f:
    c = f.read(1)
    factor, c = parse_factor(c, f)
    self.assertEqual(str(factor), test_value)
    self.assertEqual(factor._exp, 1)
    self.assertIsInstance(factor, Factor)
    self.assertIsInstance(factor._value, expected_type)

  with io.StringIO(f_value + "^-2") as f:
    c = f.read(1)
    factor, c = parse_factor(c, f)
    self.assertEqual(str(factor), test_value + "^-2")
    self.assertEqual(factor._exp, -2)
    self.assertIsInstance(factor, Factor)
    self.assertIsInstance(factor._value, expected_type)

  with io.StringIO(f_value + "^-1") as f:
    c = f.read(1)
    factor, c = parse_factor(c, f)
    self.assertEqual(str(factor), test_value + "^-1")
    self.assertEqual(factor._exp, -1)
    self.assertIsInstance(factor, Factor)
    self.assertIsInstance(factor._value, expected_type)

  with io.StringIO(f_value + "^2") as f:
    c = f.read(1)
    factor, c = parse_factor(c, f)
    self.assertEqual(str(factor), test_value + "^2")
    self.assertEqual(factor._exp, 2)
    self.assertIsInstance(factor, Factor)
    self.assertIsInstance(factor._value, expected_type)



  with io.StringIO(f_value + "   ^2") as f:
    c = f.read(1)
    factor, c = parse_factor(c, f)
    self.assertEqual(str(factor), test_value + "^2")
    self.assertEqual(factor._exp, 2)
    self.assertIsInstance(factor, Factor)
    self.assertIsInstance(factor._value, expected_type)

  with io.StringIO("   " + f_value) as f:
    c = f.read(1)
    with self.assertRaises(RuntimeError):
      factor, c = parse_factor(c, f)

  with io.StringIO("   " + f_value + "^2") as f:
    c = f.read(1)
    with self.assertRaises(RuntimeError):
      factor, c = parse_factor(c, f)


class TestAdd(unittest.TestCase):
  """
  Test the add function from the mymath library
  """

  def testNumber(self):
    """
    Test the Number class
    """
    num = Number()
    self.assertEqual(str(num), "0")

    with self.assertRaises(ValueError):
      num = Number(" ")

    num = Number("")
    self.assertEqual(str(num), "0")

    num = Number(1)
    self.assertEqual(str(num), "1")

    num = Number(-1)
    self.assertEqual(str(num), "-1")

    num = Number("12345")
    self.assertEqual(str(num), "12345")

    num = Number("-12345")
    self.assertEqual(str(num), "-12345")

    num = Number("-12345")
    num.add("67");
    self.assertEqual(str(num), "-1234567")

    num.add(8);
    self.assertEqual(str(num), "-12345678")

    with self.assertRaises(ValueError):
      num.add(-1)

    with self.assertRaises(ValueError):
      num.add("-1")

    with self.assertRaises(ValueError):
      num.add("b")

    with self.assertRaises(ValueError):
      num.add("2b")

    with self.assertRaises(ValueError):
      num.add("b2")

    with self.assertRaises(ValueError):
      num.add("")

  def testSymbol(self):
    """
    test the Synbol class
    """

    # should be string
    with self.assertRaises(TypeError): sym = Symbol(1)

    # may not start with number
    with self.assertRaises(ValueError):
      sym = Symbol("1b")

    # may not start with non_alpha char
    with self.assertRaises(ValueError):
      sym = Symbol("_b1")

    with self.assertRaises(ValueError):
      sym = Symbol("a!")

    sym = Symbol("a")
    self.assertEqual(str(sym), "a")

    # Fine if other characters are numeric, also initialize with argument works
    sym = Symbol("a1")
    self.assertEqual(str(sym), "a1")

    sym.add("b")
    self.assertEqual(str(sym), "a1b")

    # don't allow empty strings for adding
    with self.assertRaises(ValueError):
      sym.add("")

    # add must have an argument
    with self.assertRaises(TypeError):
      sym.add()

    # initialize with no argument
    sym = Symbol()
    self.assertEqual(str(sym), "")

    # add leading int if symbol still empty string
    with self.assertRaises(ValueError):
      sym.add("1a")

    # Same, but with type int instead of string
    with self.assertRaises(ValueError):
      sym.add(543)

    # valid add to empty string and then thereafter alphanumeric
    sym.add("a")
    sym.add("1a")
    self.assertEqual(str(sym), "a1a")

    # add type integer
    sym.add(543)
    self.assertEqual(str(sym), "a1a543")

    with self.assertRaises(ValueError):
      sym.add(-543)

    with self.assertRaises(ValueError):
      sym.add("543!")

  def testFactor(self):

    # when instantializing with no argument, it represents the multiplicative
    # identity, i.e. '1'
    factor = Factor()
    self.assertEqual(str(factor), "1")

    testFactor_init(self, Symbol("SomeSymbol"), "SomeSymbol")

    testFactor_init(self, Number("1234"), "1234")

    testFactor_init(self, Bracket(Expression()), "(0)")

    with self.assertRaises(TypeError):
      factor = Factor("asdf")

  def testTerm(self):

    with self.assertRaises(TypeError):
      term = Term(1)
    term = Term()
    self.assertEqual(str(term), "1")

    factor1 = Factor(Symbol("OneRingTo"))
    factor2 = Factor(Symbol("RuleThemAll"), 10)
    factor3 = Factor(Number(20))
    factor4 = Factor(Bracket(Expression().append_term(Term().append_factor(Number(2)))))

    with self.assertRaises(TypeError):
      term.append_factor("hi")

    term.append_factor(Number(5))
    self.assertEqual(str(term), "5")

    term.append_factor(Symbol("yes"))
    self.assertEqual(str(term), "5*yes")

    term.append_factor(Bracket(Expression().append_term(Term().append_factor(Symbol("sir")))))
    self.assertEqual(str(term), "5*yes*(sir)")

    term.append_factor(factor1)
    self.assertEqual(str(term), "5*yes*(sir)*OneRingTo")

    term = Term().append_factor(factor2)
    self.assertEqual(str(term), "RuleThemAll^10")

    term = Term().append_factor(factor1) \
                 .append_factor(factor2) \
                 .append_factor(factor3) \
                 .append_factor(factor4)
    self.assertEqual(str(term), "OneRingTo*RuleThemAll^10*20*(2)")
    factor = Factor(Number(20), -2)
    term.append_factor(factor)
    self.assertEqual(str(term), "OneRingTo*RuleThemAll^10*20*(2)/20^2")

    term.set_sign("+")
    self.assertEqual(str(term), "OneRingTo*RuleThemAll^10*20*(2)/20^2")

    term.set_sign("-")
    self.assertEqual(str(term), " - OneRingTo*RuleThemAll^10*20*(2)/20^2")

    term.set_sign("+")
    self.assertEqual(str(term), "OneRingTo*RuleThemAll^10*20*(2)/20^2")

    term.set_sign("-")
    self.assertEqual(str(term), " - OneRingTo*RuleThemAll^10*20*(2)/20^2")

    with self.assertRaises(ValueError):
      term.set_sign(" -")

    with self.assertRaises(ValueError):
      term.set_sign("")

    with self.assertRaises(ValueError):
      term.set_sign("a")

  def testExpression(self):
    term1 = Term().append_factor(Factor(Symbol("OneRingTo")))
    term2 = Term().append_factor(Factor(Symbol("RuleThemAll"), 10))
    term3 = Term().append_factor(Factor(Number(20)))
    term4 = Term().append_factor(
        Factor(Bracket(Expression().append_term(Term().append_factor(Number(2))))))

    express = Expression()
    self.assertEqual(str(express), "0")

    express = Expression().append_term(term1)
    self.assertEqual(str(express), "OneRingTo")

    express = Expression().append_term(term1.set_sign("-"))
    self.assertEqual(str(express), " - OneRingTo")

    express = Expression().append_term(term1.set_sign("-"))
    express.append_term(term2.set_sign("-"))
    self.assertEqual(str(express), " - OneRingTo - RuleThemAll^10")

    express._terms[0].set_sign("+")
    self.assertEqual(str(express), "OneRingTo - RuleThemAll^10")

    express.append_term(term3)
    self.assertEqual(str(express), "OneRingTo - RuleThemAll^10 + 20")

    with self.assertRaises(TypeError):
      express.append_term("Gollum")

  def testBracket(self):

    bracket = Bracket()
    self.assertEqual(str(bracket), "(0)")

    bracket = Bracket(Expression())
    self.assertEqual(str(bracket), "(0)")

    bracket = Bracket(Expression().append_term(Term().append_factor(Number(20))))
    self.assertEqual(str(bracket), "(20)")

    bracket = Bracket(Number(5))
    self.assertEqual(str(bracket), "(5)")

    bracket = Bracket(Symbol("house"))
    self.assertEqual(str(bracket), "(house)")

    with self.assertRaises(TypeError):
      bracket = Bracket("horse")

  def testParseBlank(self):
    with io.StringIO("   \t\n\n hi \n !") as f:
      c = f.read(1)
      c = parse_blank(c, f)
      self.assertEqual(c, "h")
      c = f.read(1)
      self.assertEqual(c, "i")
      c = f.read(1)

      c = parse_blank(c, f)
      self.assertEqual(c, "!")

      c = parse_blank(c, f)
      self.assertEqual(c, "!")

  def testParseBracket(self):
    with io.StringIO("(house)") as f:
      c = f.read(1)
      bracket, c  = parse_bracket(c, f)
      self.assertEqual(str(bracket), "(house)")
      self.assertIsInstance(bracket, Bracket)

    with io.StringIO("(   house + pizza *  cheese ^  -2  )") as f:
      c = f.read(1)
      bracket, c  = parse_bracket(c, f)
      self.assertEqual(str(bracket), "(house + pizza/cheese^2)")
      self.assertIsInstance(bracket, Bracket)

    with io.StringIO(" (a)") as f:
      c = f.read(1)
      with self.assertRaises(RuntimeError):
        bracket, c  = parse_bracket(c, f)

    with io.StringIO("()") as f:
      c = f.read(1)
      with self.assertRaises(RuntimeError):
        bracket, c  = parse_bracket(c, f)

    with io.StringIO("house)") as f:
      c = f.read(1)
      with self.assertRaises(RuntimeError):
        bracket, c  = parse_bracket(c, f)

    with io.StringIO("(house") as f:
      c = f.read(1)
      with self.assertRaises(RuntimeError):
        bracket, c  = parse_bracket(c, f)

  def testParseNumber(self):
    with io.StringIO("1234") as f:
      c = f.read(1)
      num, c = parse_number(c, f)
      self.assertEqual(str(num), "1234")
      self.assertIsInstance(num, Number)

    with io.StringIO("-1234") as f:
      c = f.read(1)
      num, c = parse_number(c, f)
      self.assertEqual(str(num), "-1234")
      self.assertIsInstance(num, Number)

    with io.StringIO("+1234") as f:
      c = f.read(1)
      num, c = parse_number(c, f)
      self.assertEqual(str(num), "1234")
      self.assertIsInstance(num, Number)

    with io.StringIO("1234     pizza") as f:
      c = f.read(1)
      num, c = parse_number(c, f)
      self.assertEqual(str(num), "1234")
      self.assertIsInstance(num, Number)
      self.assertEqual(c, "p")

    with io.StringIO("1234     polyjuice") as f:
      c = f.read(1)
      num, c = parse_number(c, f)
      self.assertEqual(str(num), "1234")
      self.assertIsInstance(num, Number)
      self.assertEqual(c, "p")


    with io.StringIO("  123") as f:
      c = f.read(1)
      with self.assertRaises(RuntimeError):
        num, c  = parse_number(c, f)
      c = parse_blank(c, f)
      num, c  = parse_number(c, f)
      self.assertEqual(str(num), "123")
      self.assertIsInstance(num, Number)

    with io.StringIO("p123") as f:
      c = f.read(1)
      with self.assertRaises(RuntimeError):
        num, c  = parse_number(c, f)


  def testParseSymbol(self):

    with io.StringIO("p123") as f:
      c = f.read(1)
      symbol, c = parse_symbol(c, f)
      self.assertEqual(str(symbol), "p123")
      self.assertIsInstance(symbol, Symbol)

    with io.StringIO("p123   word") as f:
      c = f.read(1)
      symbol, c = parse_symbol(c, f)
      self.assertEqual(str(symbol), "p123")
      self.assertIsInstance(symbol, Symbol)
      self.assertEqual(c, "w")

    with io.StringIO("   p123") as f:
      c = f.read(1)
      with self.assertRaises(RuntimeError):
        symbol, c = parse_symbol(c, f)

    with io.StringIO("123") as f:
      c = f.read(1)
      with self.assertRaises(RuntimeError):
        symbol, c = parse_symbol(c, f)

    with io.StringIO("!ab") as f:
      c = f.read(1)
      with self.assertRaises(RuntimeError):
        symbol, c = parse_symbol(c, f)

    with io.StringIO("(a^2 + (1 - x)^2 + (2 + y))") as f:
      c = f.read(1)
      with self.assertRaises(RuntimeError):
        symbol, c = parse_symbol(c, f)

    with io.StringIO("") as f:
      c = f.read(1)
      with self.assertRaises(RuntimeError):
        symbol, c = parse_symbol(c, f)


  def testParseFactor(self):

    testParseFactorType( self
                       , "(a^2 + (1 - x)^2 + (2 + y))"
                       , "(a^2 + (1 - x)^2 + (2 + y))"
                       , Bracket
                       )

    testParseFactorType( self
                       , "(a^2+(1- x)^2 + (2 + y))"
                       , "(a^2 + (1 - x)^2 + (2 + y))"
                       , Bracket
                       )

    testParseFactorType( self
                       , "PrancingPony"
                       , "PrancingPony"
                       , Symbol
                       )

    testParseFactorType( self
                       , "123"
                       , "123"
                       , Number
                       )

    with io.StringIO("(word") as f:
      c = f.read(1)
      with self.assertRaises(RuntimeError):
        factor, c = parse_factor(c, f)

    with io.StringIO("*(word)") as f:
      c = f.read(1)
      with self.assertRaises(RuntimeError):
        factor, c = parse_factor(c, f)

  def testParseTerm(self):

    with io.StringIO("  Apple") as f:
      c = f.read(1)
      with self.assertRaises(RuntimeError):
        term, c = parse_term(c, f)

    with io.StringIO("Apple") as f:
      c = f.read(1)
      term, c = parse_term(c, f)
      self.assertEqual(str(term), "Apple")
      self.assertIsInstance(term, Term)
      self.assertIsInstance(term._factors[0], Factor)
      self.assertIsInstance(term._factors[0]._value, Symbol)
      self.assertEqual(term._factors[0]._exp, 1)

    with io.StringIO("Apple^2") as f:
      c = f.read(1)
      term, c = parse_term(c, f)
      self. assertEqual(str(term), "Apple^2")
      self.assertIsInstance(term, Term)
      self.assertIsInstance(term._factors[0], Factor)
      self.assertIsInstance(term._factors[0]._value, Symbol)
      self.assertEqual(term._factors[0]._exp, 2)

    with io.StringIO("Apple^-2") as f:
      c = f.read(1)
      term, c = parse_term(c, f)
      self. assertEqual(str(term), "1/Apple^2")
      self.assertIsInstance(term, Term)
      self.assertIsInstance(term._factors[0], Factor)
      self.assertIsInstance(term._factors[0]._value, Symbol)
      self.assertEqual(term._factors[0]._exp, -2)

    with io.StringIO("Apple^-1") as f:
      c = f.read(1)
      term, c = parse_term(c, f)
      self. assertEqual(str(term), "1/Apple")
      self.assertIsInstance(term, Term)
      self.assertIsInstance(term._factors[0], Factor)
      self.assertIsInstance(term._factors[0]._value, Symbol)
      self.assertEqual(term._factors[0]._exp, -1)

    with io.StringIO("1/Apple") as f:
      c = f.read(1)
      term, c = parse_term(c, f)
      self. assertEqual(str(term), "1/Apple")
      self.assertIsInstance(term, Term)
      self.assertIsInstance(term._factors[1], Factor)
      self.assertIsInstance(term._factors[1]._value, Symbol)
      self.assertEqual(term._factors[1]._exp, -1)

    with io.StringIO("123 / Apple ^ 2 * (x+y) + Pear") as f:
      c = f.read(1)
      term, c = parse_term(c, f)
      self.assertEqual(str(term), "123/Apple^2*(x + y)")
      self.assertIsInstance(term, Term)
      self.assertIsInstance(term._factors[0], Factor)
      self.assertIsInstance(term._factors[0]._value, Number)
      self.assertEqual(term._factors[0]._exp, 1)
      self.assertIsInstance(term._factors[1], Factor)
      self.assertIsInstance(term._factors[1]._value, Symbol)
      self.assertEqual(term._factors[1]._exp, -2)
      self.assertIsInstance(term._factors[2], Factor)
      self.assertIsInstance(term._factors[2]._value, Bracket)
      self.assertEqual(term._factors[2]._exp, 1)

    with io.StringIO("123 / Apple ^-2  + (x+y) + Pear") as f:
      c = f.read(1)
      term, c = parse_term(c, f)
      self.assertEqual(str(term), "123*Apple^2")

    with io.StringIO("(apple") as f:
      c = f.read(1)
      with self.assertRaises(RuntimeError):
        term, c = parse_term(c, f)

    # for now, only supports integer powers
    with io.StringIO("apple^pear") as f:
      c = f.read(1)
      with self.assertRaises(RuntimeError):
        term, c = parse_term(c, f)

  def testParseExpression(self):
    with io.StringIO("123 / Apple ^ 2  + (x+y)+Pear") as f:
      c = f.read(1)
      express, c = parse_expression(c, f)
      self.assertEqual(str(express), "123/Apple^2 + (x + y) + Pear")
      self.assertEqual(str(express._terms[0]), "123/Apple^2")
      self.assertEqual(str(express._terms[1]), "(x + y)")
      self.assertEqual(str(express._terms[2]), "Pear")

    with io.StringIO("   123 / Apple ^ 2  + (x+y)+Pear") as f:
      c = f.read(1)
      with self.assertRaises(RuntimeError):
        term, c = parse_expression(c, f)

    with io.StringIO("123 / Apple ^ 2  + (x+y +Pear") as f:
      c = f.read(1)
      with self.assertRaises(RuntimeError):
        term, c = parse_expression(c, f)


if __name__ == '__main__':
  unittest.main()
