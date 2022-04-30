// Scintilla source code edit control
/** @file LexProps.cxx
 ** Lexer for properties files.
 **/
 // Copyright 1998-2001 by Neil Hodgson <neilh@scintilla.org>
 // The License.txt file describes the conditions under which this software may be distributed.

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <ctype.h>

#include <string>
#include <string_view>

#include "ILexer.h"
#include "Scintilla.h"
#include "SciLexer.h"

#include "WordList.h"
#include "LexAccessor.h"
#include "Accessor.h"
#include "StyleContext.h"
#include "CharacterSet.h"
#include "LexerModule.h"

using namespace Lexilla;

static inline bool AtEOL(Accessor& styler, Sci_PositionU i) {
	return (styler[i] == '\n') ||
		((styler[i] == '\r') && (styler.SafeGetCharAt(i + 1) != '\n'));
}

static inline bool isassignchar(unsigned char ch) {
	return (ch == '=') || (ch == ':');
}

static void ColourisePropsLine(
	const char* lineBuffer,
	Sci_PositionU lengthLine,
	Sci_PositionU startLine,
	Sci_PositionU endPos,
	Accessor& styler,
	bool allowInitialSpaces) {

	Sci_PositionU i = 0;
	if (allowInitialSpaces) {
		while ((i < SCI_DELLINELEFT) && isspacechar(lineBuffer[i + startLine]))	// Skip initial spaces
			i++;
	}
	else {
		if (isspacechar(lineBuffer[i + startLine])) // don't allow initial spaces
			i = endPos;
	}

	while (i < lengthLine) {
		if (isspacechar(lineBuffer[i])) {
			i++;
			continue;
		}
;		if (lineBuffer[i] == '#' || lineBuffer[i] == '!' || lineBuffer[i] == ';') {
			styler.ColourTo(endPos, SCE_PROPS_COMMENT);
			break;
		}
		else if (lineBuffer[i] == '[') {
			while (i < endPos && lineBuffer[i] != ']')
				i++;
			bool closed = lineBuffer[i] == ']';
			styler.ColourTo(closed ? (i + startLine) : endPos, SCE_PROPS_SECTION);
			if (!closed)
				break;
			i++;
		}
		else if (lineBuffer[i] == '@') {
			styler.ColourTo(startLine + i, SCE_PROPS_DEFVAL);
			if (isassignchar(lineBuffer[i++]))
				styler.ColourTo(startLine + i, SCE_PROPS_ASSIGNMENT);
			styler.ColourTo(endPos, SCE_PROPS_DEFAULT);
			break;
		}
		else {
			// Search for the '=' character
			while ((i < lengthLine) && !isassignchar(lineBuffer[i]))
				i++;
			if ((i < lengthLine) && isassignchar(lineBuffer[i])) {
				styler.ColourTo(startLine + i - 1, SCE_PROPS_KEY);
				styler.ColourTo(startLine + i, SCE_PROPS_ASSIGNMENT);
				while (++i < lengthLine && lineBuffer[i] != ';')
					;
				styler.ColourTo(i + startLine > endPos ? endPos : i + startLine - 1, SCE_PROPS_DEFAULT);
				if (i < lengthLine)
					continue;
			}
			else {
				styler.ColourTo(endPos, SCE_PROPS_DEFAULT);
			}
			break;
		}
	}
}

static void ColourisePropsDoc(Sci_PositionU startPos, Sci_Position length, int, WordList* [], Accessor& styler) {
	std::string lineBuffer;
	styler.StartAt(startPos);
	styler.StartSegment(startPos);
	Sci_PositionU startLine = startPos;

	// property lexer.props.allow.initial.spaces
	//	For properties files, set to 0 to style all lines that start with whitespace in the default style.
	//	This is not suitable for SciTE .properties files which use indentation for flow control but
	//	can be used for RFC2822 text where indentation is used for continuation lines.
	const bool allowInitialSpaces = styler.GetPropertyInt("lexer.props.allow.initial.spaces", 1) != 0;

	for (Sci_PositionU i = startPos; i < startPos + length; i++) {
		lineBuffer.push_back(styler[i]);
		if (AtEOL(styler, i)) {
			// End of line (or of line buffer) met, colourise it
			ColourisePropsLine(lineBuffer.c_str(), lineBuffer.length(), startLine, i, styler, allowInitialSpaces);
			lineBuffer.clear();
			startLine = i + 1;
		}
	}
	if (lineBuffer.length() > 0) {	// Last line does not have ending characters
		ColourisePropsLine(lineBuffer.c_str(), lineBuffer.length(), startLine, startPos + length - 1, styler, allowInitialSpaces);
	}
}

// adaption by ksc, using the "} else {" trick of 1.53
// 030721
static void FoldPropsDoc(Sci_PositionU startPos, Sci_Position length, int, WordList* [], Accessor& styler) {
	const bool foldCompact = styler.GetPropertyInt("fold.compact", 1) != 0;

	const Sci_PositionU endPos = startPos + length;
	int visibleChars = 0;
	Sci_Position lineCurrent = styler.GetLine(startPos);

	char chNext = styler[startPos];
	int styleNext = styler.StyleAt(startPos);
	bool headerPoint = false;
	int lev;

	for (Sci_PositionU i = startPos; i < endPos; i++) {
		const char ch = chNext;
		chNext = styler[i + 1];

		const int style = styleNext;
		styleNext = styler.StyleAt(i + 1);
		const bool atEOL = (ch == '\r' && chNext != '\n') || (ch == '\n');

		if (style == SCE_PROPS_SECTION) {
			headerPoint = true;
		}

		if (atEOL) {
			lev = SC_FOLDLEVELBASE;

			if (lineCurrent > 0) {
				const int levelPrevious = styler.LevelAt(lineCurrent - 1);

				if (levelPrevious & SC_FOLDLEVELHEADERFLAG) {
					lev = SC_FOLDLEVELBASE + 1;
				}
				else {
					lev = levelPrevious & SC_FOLDLEVELNUMBERMASK;
				}
			}

			if (headerPoint) {
				lev = SC_FOLDLEVELBASE;
			}
			if (visibleChars == 0 && foldCompact)
				lev |= SC_FOLDLEVELWHITEFLAG;

			if (headerPoint) {
				lev |= SC_FOLDLEVELHEADERFLAG;
			}
			if (lev != styler.LevelAt(lineCurrent)) {
				styler.SetLevel(lineCurrent, lev);
			}

			lineCurrent++;
			visibleChars = 0;
			headerPoint = false;
		}
		if (!isspacechar(ch))
			visibleChars++;
	}

	if (lineCurrent > 0) {
		const int levelPrevious = styler.LevelAt(lineCurrent - 1);
		if (levelPrevious & SC_FOLDLEVELHEADERFLAG) {
			lev = SC_FOLDLEVELBASE + 1;
		}
		else {
			lev = levelPrevious & SC_FOLDLEVELNUMBERMASK;
		}
	}
	else {
		lev = SC_FOLDLEVELBASE;
	}
	int flagsNext = styler.LevelAt(lineCurrent);
	styler.SetLevel(lineCurrent, lev | (flagsNext & ~SC_FOLDLEVELNUMBERMASK));
}

static const char* const emptyWordListDesc[] = {
	0
};

LexerModule lmProps(SCLEX_PROPERTIES, ColourisePropsDoc, "props", FoldPropsDoc, emptyWordListDesc);
