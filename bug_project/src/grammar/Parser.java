package grammar;

import java.io.FileInputStream;
import java.io.IOException;

import errors.SyntaxError;
import lexer.Lexer;
import lexer.Tag;
import lexer.Token;

public class Parser {
	private Lexer lexer;
	private Token token;

	public Parser (FileInputStream input) {
		lexer = new Lexer(input);
	}

	private void check(int tag) throws SyntaxError, IOException {
		if (token.getTag() == tag) {
			token = lexer.scan();
		} else {
			throw new SyntaxError("Error in line " + lexer.line);
		}
	}

    public void analyze() throws SyntaxError, IOException {
      token = lexer.scan(); statementSequence();
      if (token.getTag() == Tag.EOF) {
        System.out.println("ACCEPTED");
      } else {
        throw new SyntaxError("Error in line " + lexer.line);
      }
    }

	private void statementSequence() throws SyntaxError, IOException {
		if (token.getTag() == Tag.VAR || token.getTag() == Tag.ID || token.getTag() == Tag.PRINT  ||
		    token.getTag() == Tag.WHILE || token.getTag() == (int) '{' || token.getTag() == Tag.IF) {
		statement();
		statementSequence();
		} else {
			//do nothing
		}
	}

	private void statement() throws SyntaxError, IOException {
		if (token.getTag() == Tag.VAR) {
			check(Tag.VAR);
			check(Tag.ID);
			check((int) ';');
		} else if (token.getTag() == Tag.ID) {
			check(Tag.ID);
			check((int) '=');
			expression();
			check((int) ';');
		} else if (token.getTag() == Tag.PRINT) {
			check(Tag.PRINT);
			check((int) '(');
			displayList();
			check((int) ')');
			check((int) ';');
		} else if (token.getTag() == Tag.WHILE) {
			check(Tag.WHILE);
			check((int) '(');
			eBool();
			check((int) ')');
			statement();
		} else if (token.getTag() == (int) '{') {
			check((int) '{');
			statementSequence();
			check((int) '}');
		} else if (token.getTag() == Tag.IF) {
			check(Tag.IF);
			check((int) '(');
			eBool();
			check((int) ')');
			statement();
			restIf();
		}
	}

	private void restIf() throws SyntaxError, IOException {
		if (token.getTag() == Tag.ELSE) {
			check(Tag.ELSE);
			statement();
		} else {
			// do nothing
		}
	}

	private void expression() throws SyntaxError, IOException {
		term();
		ePrime();
	}

	private void ePrime() throws SyntaxError, IOException {
		if (token.getTag() == ((int) '+')) {
			check((int) '+');
			term();
			ePrime();
		} else if (token.getTag() == ((int) '-')) {
			check((int) '-');
			term();
			ePrime();
		} else {
			// do nothing
		}
	}

	private void term() throws SyntaxError, IOException {
		factor();
		tPrime();
	}

	private void tPrime() throws SyntaxError, IOException {
		if (token.getTag() == ((int) '*')) {
			check((int) '*');
			factor();
			tPrime();
		} else if (token.getTag() == ((int) '/')) {
			check((int) '/');
			factor();
			tPrime();
		} else {
			// do nothing
		}
	}

	private void factor() throws SyntaxError, IOException {
		if (token.getTag() == Tag.ID) {
			check(Tag.ID);
		} else if (token.getTag() == Tag.NUMBER) {
			check(Tag.NUMBER);
		} else if (token.getTag() == Tag.STRING) {
			check(Tag.STRING);
		} else if (token.getTag() == ((int) '(')) {
			check((int) '(');
			expression();
			check((int) ')');
		} else {
			throw new SyntaxError();
		}
	}

	private void eBool() throws SyntaxError, IOException {
		if (token.getTag() == Tag.TRUE) {
			check(Tag.TRUE);
		} else if (token.getTag() == Tag.FALSE) {
			check(Tag.FALSE);
		} else {
			expression();
			switch (token.getTag()) {
				case Tag.EQ:
					check(Tag.EQ);
					break;
				case Tag.GEQ:
					check(Tag.GEQ);
					break;
				case Tag.LEQ:
					check(Tag.LEQ);
					break;
				case ((int)'>'):
					check((int)'>');
					break;
				case ((int)'<'):
					check((int)'<');
					break;
				default:
					throw new SyntaxError();
			}
			expression();
		}
	}

	private void displayList() throws SyntaxError, IOException {
		if (   token.getTag() == Tag.ID
				|| token.getTag() == Tag.NUMBER
				|| token.getTag() == Tag.STRING
		 ) {
			element();
			elementSequence();
		 } else {
			 // do nothing
		 }
	}

	private void element() throws SyntaxError, IOException {
		if (token.getTag() == Tag.ID) {
			check(Tag.ID);
		} else if (token.getTag() == Tag.NUMBER) {
			check(Tag.NUMBER);
		} else if (token.getTag() == Tag.STRING) {
			check(Tag.STRING);
		} else {
			throw new SyntaxError();
		}
	}

	private void elementSequence() throws SyntaxError, IOException {
		if (token.getTag() == ((int)',')) {
			check((int)',');
			element();
			elementSequence();
		} else {
			// do nothing;
		}
	}
}
