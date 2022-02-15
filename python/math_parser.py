#! /usr/bin/python3

global depth
global debug
depth = 0
debug=False

class Number:
    """Math numbers"""
    def __init__(self, value=None):
        if value:
            if isinstance(value, int):
                self._value = value
            elif isinstance(value, str):
                self._value = int(value)
        else:
            self._value = 0


    def __repr__(self):
        return str(self._value)

    #add is a misnomer. append is more appropriate
    def add(self, other):
        if isinstance(other, str):
            if other.isnumeric():
              return self.add(int(other))
            else:
              raise ValueError("numeric digits expected, found " + other)
        elif isinstance(other, int):
            if other < 0:
                raise ValueError("can only append non negative digits/numbers")

            self._value = int(str(self._value) + str(other))
            return self
        else:
            raise RuntimeError("Can only append type 'str' or 'int'")


# TODO: allow - and _ in symbol names
class Symbol:
    """Math symbols(variables)"""
    def __init__(self, name=None):
        self._sign = ""
        if name:
            if not isinstance(name, str):
                raise TypeError("Symbols should take str as optional name argument")
            if not name[0].isalpha():
                raise ValueError("Symbols should start with an alphabetic symbol")
            if not name.isalnum():
                raise ValueError("Symbols may only consist of alphanumeric characters")
            self._name = name
        else:
            self._name = ""

    def __repr__(self):
        return self._sign + self._name

    #add is a misnomer. append is more appropriate
    def add(self, other):
        if isinstance(other, str):
            if len(other) == 0:
                raise ValueError("please provide a non empty string as argument")
            if not other.isalnum():
                raise ValueError("please provide only alphanumeric characters for symbol")
            if len(self._name) == 0 and not other[0].isalpha():
                raise ValueError("Symbols should start with an alphabetic symbol")
            self._name += other
            return self
        elif isinstance(other, int):
            return self.add(str(other))

class Factor:
    """Math/polynomial factors"""
    def __init__(self, substance=None, exponent=None):
        self._sign = "" #TODO remove, as this is handle on term level

        if substance:
          if (isinstance(substance, Symbol) or
             isinstance(substance, Number) or
             isinstance(substance, Bracket)):
            self._value = substance
          else:
            raise TypeError("Factor may only be instantialized with a Symbol,"
                             " a Number or a Bracket as argument")
        else:
          dbg_print("Warning, this is unusual and unsafe")
          self._value = Number(1)

        if exponent:
          if isinstance(exponent, Number):
            self._exp = int(str(exponent))
          elif isinstance(exponent, int):
            self._exp = exponent
          else:
            raise TypeError("The exponent of a Factor may only be initialized "
                             "with a Number or an int")
        else:
          self._exp = 1


    def __repr__(self):
        if self._exp == 1:
            return self._sign + str(self._value)
        else:
            return self._sign + str(self._value) + "^" + str(self._exp)

class Term:
    """Math/polynomial terms"""
    def __init__(self):
        self._sign = ""
        self._factors=[]

    def __repr__(self):
        if len(self._factors) == 0:
            return "1"
        count = 0
        term = ""
        for factor in self._factors:
            sfactor = str(factor._value)
            if not abs(factor._exp) == 1:
                sfactor += "^" + str(abs(factor._exp))
            if (factor._sign == "-"):
                sfactor = "(-" + sfactor + ")"

            if count == 0:
                if factor._exp < 0:
                    term += "1/" + sfactor
                else:
                    term += sfactor
                count += 1
            else:
                if factor._exp < 0:
                    term += "/" + sfactor
                else:
                    term += "*" + sfactor
        if self._sign == "-":
            term = " - " + term
        return term

    def set_sign(self, sign):
      if not isinstance(sign, str):
        raise TypeError("sign should be set with a string")

      if not len(sign) == 1:
        raise ValueError("Sign should consist of a single character")

      if not sign == "+" and not sign == "-":
        raise ValueError("Sign should be either '+' or '-'")

      if sign == "+":
        self._sign = ""
      elif sign == "-":
        self._sign = "-"
      else:
        raise RuntimeError("This is not supposed to be possible")
      return self
      
    def append_factor(self, other):
        if isinstance(other, Factor):
          self._factors.append(other)
          return self
        if isinstance(other, Number) or isinstance(other, Symbol) or isinstance(other, Bracket):
          self.append_factor(Factor(other))
          return self
        else:
            raise TypeError("can only append Factors or Factor compatible objects to terms")


class Expression:
    """Polynomial expression"""
    def __init__(self):
        self._terms=[]

    def __repr__(self):
        if len(self._terms) == 0:
            return "0"
        count = 0
        expression = ""
        for term in self._terms:
            sterm = str(term)
            if count == 0:
                count += 1
                expression += sterm
            else:
                if term._sign == "-":
                    expression += sterm
                else:
                    expression += " + " + sterm
        return expression

    def append_term(self, other):
        if isinstance(other, Term):
            self._terms.append(other)
            return self
        else:
            raise TypeError("can only append instances of type Term to Expression")

class Bracket:
    """Expression surrounded by brackets"""
    def __init__(self, expr=None):
      if not expr:
        self._expr = Expression()
      elif isinstance(expr, Expression):
        self._expr = expr
      elif isinstance(expr, Number) or isinstance(expr, Symbol) or \
           isinstance(expr, Bracket):
        self._expr = Expression().append_term(Term().append_factor(expr))
      else:
        raise TypeError("bracket requires Expression, Number, Symbol, Bracket")

    def __repr__(self):
        return "(" + str(self._expr) +  ")"


def dbg_on():
    global debug
    debug = True

def dbg_off():
    global debug
    debug = False

def dbg_enter(fname):
    global depth
    depth += 1
    dbg_print("+" + fname)

def dbg_leave(fname):
    global depth
    dbg_print("-" + fname)
    depth -= 1


def dbg_print(message):
    global debug
    if debug:
        print(depth_blank() + str(message))

def depth_blank():
    global depth
    blank = ""
    for i in range(depth):
        blank += "  "
    return blank

def parse_blank(c, f):
    while c.isspace():
        c = f.read(1)
    return c

def parse_bracket(c, f):
    dbg_enter("parse_bracket")

    if not c == "(":
        raise RuntimeError("incorrect char for bracket, got '" + c + "', not '('")

    # get the bracket
    c = f.read(1);
    c = parse_blank(c, f)

    express, c = parse_expression(c, f)
    sbracket = Bracket(express)

    if not c == ")":
        raise RuntimeError("error: badly formed brackets, no closing ')'")
    c = f.read(1);
    c = parse_blank(c, f)

    dbg_leave("parse_bracket")
    return sbracket, c

def parse_number(c, f):
    dbg_enter("parse_number")

    if not c.isnumeric() and not c == "+" and not c == "-":
        raise RuntimeError ("Number expects to start with a number, got '" + c + "' instead")

    sign = ""
    if c == "+" or c == "-":
        sign = c
        c = f.read(1)
        c = parse_blank(c, f)

    snum = ""
    while c.isnumeric():
        snum += c
        c = f.read(1)
    c = parse_blank(c, f)

    cnum = Number(sign + snum)


    dbg_leave("parse_number")
    return cnum, c

def parse_symbol(c, f):
    dbg_enter("parse_symbol")

    if not c.isalpha():
      raise RuntimeError("Symbols start with alphabetic characters")

    cvar = Symbol()
    while c.isalnum():
        cvar = cvar.add(c)
        c = f.read(1)
    c = parse_blank(c, f)

    dbg_leave("parse_symbol")
    return cvar, c

def parse_factor(c, f):
    dbg_enter ("parse_factor")

    if c.isalpha():
        substance, c = parse_symbol(c, f)
        c = parse_blank(c, f)
    elif c.isnumeric():
        substance, c = parse_number(c, f)
        c = parse_blank(c, f)
    elif c == "(":
        substance, c = parse_bracket(c, f)
        c = parse_blank(c, f)
    else:
        raise RuntimeError ("Should not encounter '" + c + "' at the start of a factor")

    # just to be sure
    c = parse_blank(c, f)

    # TODO: handle cases where exponent has negative number
    if c == "^":
        c = f.read(1)
        c = parse_blank(c, f)
        exponent, c = parse_number(c, f)
    else:
      exponent=1


    # just to be sure
    c = parse_blank(c, f)

    cfactor = Factor(substance=substance, exponent=exponent);

    dbg_leave ("parse_factor")
    return cfactor, c


def parse_term(c, f):
    dbg_enter ("parse_term")

    cterm = Term()

    if c == "+" or c == "-":
        cterm.set_sign(c)
        c = f.read(1)
        c = parse_blank(c, f)

    factor, c = parse_factor(c, f)
    cterm = cterm.append_factor(factor)

    while c == "*" or c == "/":
        op=c
        c = f.read(1)
        c = parse_blank(c, f)

        factor, c = parse_factor(c, f)
        c = parse_blank(c, f)

        if op == "/":
            factor._exp *= -1
        cterm.append_factor(factor)

    dbg_leave ("parse_term")
    return cterm, c

def parse_expression(c, f):
    dbg_enter("parse_expression")

    express = ""
    cexpress = Expression()

    term, c = parse_term(c, f)
    express += str(term)
    cexpress = cexpress.append_term(term)

    c = parse_blank(c, f)

    while c == "+" or c == "-":
        op = c
        term, c = parse_term(c, f)
        c = parse_blank(c, f)
        if op == "+":
            express += op + str(term)
        else:
            express += str(term)
        cexpress = cexpress.append_term(term)

    dbg_leave("parse_expression")
    return cexpress, c

