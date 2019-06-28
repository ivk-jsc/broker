/*
 * Copyright 2014-present IVK JSC. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
package com.broker.itest.jms.selector;

import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

import javax.jms.InvalidSelectorException;
import javax.jms.JMSException;

import org.junit.Ignore;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TestName;

import com.broker.itest.testcase.QueueConnectionTestCase;

/**
 * Test the syntax of of message selector of JMS
 */
public class SelectorSyntaxTest extends QueueConnectionTestCase {

	@Rule
	public TestName name = new TestName();

	@Override
	public String getTestName() {
		return name.getMethodName();
	}

	/**
	 * Test that identifiers that start with a valid Java identifier start character are valid. A valid identifier means
	 * that the method <code>Character.isJavaIdentifierStart</code> returns <code>true</code> for this identifier first
	 * character.
	 * 
	 * @see <a href="http://java.sun.com/j2se/1.3/docs/api/java/lang/Character.html#isJavaIdentifierStart(char)">
	 *      Character.isJavaIdentifierStart(char)</a>
	 */
	@Test
	public void testValidIdentifiersStart() {
		String identifier = null;
		try {
			identifier = "_correct";
			assertTrue(identifier + " starts with an invalid Java identifier start character", Character.isJavaIdentifierStart(identifier.charAt(0)));
			receiver.close();
			receiver = receiverSession.createReceiver(receiverQueue, identifier + " IS NULL");

			identifier = "$correct";
			assertTrue(identifier + " starts with an invalid Java identifier start character", Character.isJavaIdentifierStart(identifier.charAt(0)));
			receiver.close();
			receiver = receiverSession.createReceiver(receiverQueue, identifier + " IS NULL");
		} catch (JMSException e) {
			fail(identifier + " is a correct identifier. \n" + e);
		}
	}

	/**
	 * Test that identifiers that start with an invalid Java identifier start character are invalid.
	 * 
	 * @see #testValidIdentifiersStart()
	 */
	@Test
	public void testInvalidIdentifiersStart() {
		String identifier = null;
		try {
			identifier = "1uncorrect";
			assertTrue(identifier + " starts with an invalid Java identifier start character", !Character.isJavaIdentifierStart(identifier.charAt(0)));
			receiver.close();
			receiver = receiverSession.createReceiver(receiverQueue, identifier + " IS NULL");
			fail(identifier + " starts with an invalid Java identifier start character");
		} catch (JMSException e) {
		}

		try {
			identifier = "%uncorrect";

			assertTrue(identifier + " starts with an invalid Java identifier start character", !Character.isJavaIdentifierStart(identifier.charAt(0)));
			receiver = receiverSession.createReceiver(receiverQueue, identifier + " IS NULL");
			fail(identifier + " starts with an invalid Java identifier start character");
		} catch (JMSException e) {
		}

	}

	/**
	 * Test that message selector can be an empty string.
	 */
	@Test
	public void testEmptyStringAsSelector() {
		try {
			//need close previous receiver for this session
			receiver.close();
			receiver = receiverSession.createReceiver(receiverQueue, "");
		} catch (InvalidSelectorException e) {
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test that identifiers can't be <code>NULL</code>.
	 */
	@Test
	public void testIdentifierNULL() {
		try {
			receiver.close();
			receiver = receiverSession.createReceiver(receiverQueue, "NULL = ZERO");
			fail("NULL is not a valid identifier");
		} catch (InvalidSelectorException e) {
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	@Test
	@Ignore
	/**
	 * Test that identifiers can't be <code>TRUE</code>.
	 */
	public void testIdentifierTRUE() {
		try {
			receiver.close();
			receiver = receiverSession.createReceiver(receiverQueue, "TRUE > 0");
			fail("TRUE is not a valid identifier");
		} catch (InvalidSelectorException e) {
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	@Test
	@Ignore
	/**
	 * Test that identifiers can't be <code>FALSE</code>.
	 */
	public void testIdentifierFALSE() {
		try {
			receiver.close();
			receiver = receiverSession.createReceiver(receiverQueue, "FALSE > 0");
			fail("FALSE is not a valid identifier");
		} catch (InvalidSelectorException e) {
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test that identifiers can't be <code>NOT</code>.
	 */
	@Test
	public void testIdentifierNOT() {
		try {
			receiver.close();
			receiver = receiverSession.createReceiver(receiverQueue, "NOT > 0");
			fail("NOT is not a valid identifier");
		} catch (InvalidSelectorException e) {
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test that identifiers can't be <code>AND</code>.
	 */
	@Test
	public void testIdentifierAND() {
		try {
			receiver.close();
			receiver = receiverSession.createReceiver(receiverQueue, "AND > 0");
			fail("AND is not a valid identifier");
		} catch (InvalidSelectorException e) {
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test that identifiers can't be <code>OR</code>.
	 */
	@Test
	public void testIdentifierOR() {
		try {
			receiver.close();
			receiver = receiverSession.createReceiver(receiverQueue, "OR > 0");
			fail("OR is not a valid identifier");
		} catch (InvalidSelectorException e) {
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test that identifiers can't be <code>BETWEEN</code>.
	 */
	@Test
	public void testIdentifierBETWEEN() {
		try {
			receiver.close();
			receiver = receiverSession.createReceiver(receiverQueue, "BETWEEN > 0");
			fail("BETWEEN is not a valid identifier");
		} catch (InvalidSelectorException e) {
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test that identifiers can't be <code>LIKE</code>.
	 */
	@Test
	public void testIdentifierLIKE() {
		try {
			receiver.close();
			receiver = receiverSession.createReceiver(receiverQueue, "LIKE > 0");
			fail("LIKE is not a valid identifier");
		} catch (InvalidSelectorException e) {
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test that identifiers can't be <code>IN</code>.
	 */
	@Test
	public void testIdentifierIN() {
		try {
			receiver.close();
			receiver = receiverSession.createReceiver(receiverQueue, "IN > 0");
			fail("IN is not a valid identifier");
		} catch (InvalidSelectorException e) {
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test that identifiers can't be <code>IS</code>.
	 */
	@Test
	public void testIdentifierIS() {
		try {
			receiver.close();
			receiver = receiverSession.createReceiver(receiverQueue, "IS > 0");
			fail("IS is not a valid identifier");
		} catch (InvalidSelectorException e) {
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test that identifiers can't be <code>ESCAPE</code>.
	 */
	@Test
	public void testIdentifierESCAPE() {
		try {
			receiver.close();
			receiver = receiverSession.createReceiver(receiverQueue, "ESCAPE > 0");
			fail("ESCAPE is not a valid identifier");
		} catch (InvalidSelectorException e) {
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test syntax of "<em>identifier</em> IS [NOT] NULL"
	 */
	@Test
	public void testNull() {
		try {
			receiver.close();
			receiver = receiverSession.createReceiver(receiverQueue, "prop_name IS NULL");
			receiver.close();
			receiver = receiverSession.createReceiver(receiverQueue, "prop_name IS NOT NULL");
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test syntax of "<em>identifier</em> [NOT] LIKE <em>pattern-value</em> [ESCAPE <em>escape-character</em>]"
	 */
	@Test
	public void testLike() {
		try {
			receiver.close();
			receiver = receiverSession.createReceiver(receiverQueue, "phone LIKE '12%3'");
			receiver.close();
			receiver = receiverSession.createReceiver(receiverQueue, "word LIKE 'l_se'");
			receiver.close();
			receiver = receiverSession.createReceiver(receiverQueue, "underscored LIKE '\\_%' ESCAPE '\\'");
			receiver.close();
			receiver = receiverSession.createReceiver(receiverQueue, "phone NOT LIKE '12%3'");
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test syntax of "<em>identifier</em> [NOT] IN (<em>string-literal1</em>, <em>string-literal2</em>,...)"
	 */
	@Test
	public void testIn() {
		try {
			receiver.close();
			receiver = receiverSession.createReceiver(receiverQueue, "Country IN ('UK', 'US', 'France')");
			receiver.close();
			receiver = receiverSession.createReceiver(receiverQueue, "Country NOT IN ('UK', 'US', 'France')");
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test syntax of "<em>arithmetic-expr1</em> [NOT] BETWEEN <em>arithmetic-expr2</em> and <em>arithmetic-expr3</em>"
	 */
	@Test
	public void testBetween() {
		try {
			receiver.close();
			receiver = receiverSession.createReceiver(receiverQueue, "age BETWEEN 15 and 19");
			receiver.close();
			receiver = receiverSession.createReceiver(receiverQueue, "age NOT BETWEEN 15 and 19");
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test diffent syntax for approximate numeric literal (+6.2, -95.7, 7.)
	 */
	@Test
	public void testApproximateNumericLiteral() {
		try {
			receiver.close();
			receiver = receiverSession.createReceiver(receiverQueue, "average = +6.2");
			receiver.close();
			receiver = receiverSession.createReceiver(receiverQueue, "average = -95.7");
			receiver.close();
			receiver = receiverSession.createReceiver(receiverQueue, "average = 7.");
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test diffent syntax for exact numeric literal (+62, -957, 57)
	 */
	@Test
	public void testExactNumericLiteral() {
		try {
			receiver.close();
			receiver = receiverSession.createReceiver(receiverQueue, "average = +62");
			receiver.close();
			receiver = receiverSession.createReceiver(receiverQueue, "max = -957");
			receiver.close();
			receiver = receiverSession.createReceiver(receiverQueue, "max = 57");
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test diffent syntax for zero as an exact or an approximate numeric literal (0, 0.0, 0.)
	 */
	@Test
	public void testZero() {
		try {
			receiver.close();
			receiver = receiverSession.createReceiver(receiverQueue, "max = 0");
			receiver.close();
			receiver = receiverSession.createReceiver(receiverQueue, "max = 0.0");
			receiver.close();
			receiver = receiverSession.createReceiver(receiverQueue, "max = 0.");
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}

	/**
	 * Test diffent syntax for string literal ('literal' and 'literal''s')
	 */
	@Test
	public void testString() {
		try {
			receiver.close();
			receiver = receiverSession.createReceiver(receiverQueue, "string = 'literal'");
			receiver.close();
			receiver = receiverSession.createReceiver(receiverQueue, "string = 'literal''s'");
		} catch (JMSException e) {
			fail(e.getMessage());
		}
	}
}
