/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/UI/UIEdit.h"
#include "engines/wintermute/UI/UIObject.h"
#include "engines/wintermute/UI/UITiledImage.h"
#include "engines/wintermute/utils/StringUtil.h"
#include "engines/wintermute/Base/BActiveRect.h"
#include "engines/wintermute/Base/BFileManager.h"
#include "engines/wintermute/Base/BFont.h"
#include "engines/wintermute/Base/BFontStorage.h"
#include "engines/wintermute/Base/BKeyboardState.h"
#include "engines/wintermute/Base/BDynBuffer.h"
#include "engines/wintermute/Base/BParser.h"
#include "engines/wintermute/Base/BSprite.h"
#include "engines/wintermute/Base/BStringTable.h"
#include "engines/wintermute/Base/BGame.h"
#include "engines/wintermute/PlatformSDL.h"
#include "engines/wintermute/Base/scriptables/ScValue.h"
#include "engines/wintermute/Base/scriptables/ScStack.h"
#include "engines/wintermute/Base/scriptables/ScScript.h"
#include "engines/wintermute/utils/utils.h"
#include "common/util.h"
#include "common/keyboard.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CUIEdit, false)

//////////////////////////////////////////////////////////////////////////
CUIEdit::CUIEdit(CBGame *inGame): CUIObject(inGame) {
	_type = UI_EDIT;

	_fontSelected = NULL;

	_selStart = _selEnd = 10000;
	_scrollOffset = 0;

	_cursorChar = NULL;
	setCursorChar("|");

#ifdef __WIN32__
	_cursorBlinkRate = GetCaretBlinkTime();
#else
	_cursorBlinkRate = 600;
#endif
	_frameWidth = 0;

	setText("");

	_lastBlinkTime = 0;
	_cursorVisible = true;

	_maxLength = -1;

	_canFocus = true;
}


//////////////////////////////////////////////////////////////////////////
CUIEdit::~CUIEdit() {
	if (!_sharedFonts) {
		if (_fontSelected)   Game->_fontStorage->removeFont(_fontSelected);
	}

	delete[] _cursorChar;
	_cursorChar = NULL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIEdit::loadFile(const char *filename) {
	byte *buffer = Game->_fileManager->readWholeFile(filename);
	if (buffer == NULL) {
		Game->LOG(0, "CUIEdit::LoadFile failed for file '%s'", filename);
		return E_FAIL;
	}

	HRESULT ret;

	_filename = new char [strlen(filename) + 1];
	strcpy(_filename, filename);

	if (FAILED(ret = loadBuffer(buffer, true))) Game->LOG(0, "Error parsing EDIT file '%s'", filename);

	delete [] buffer;

	return ret;
}


TOKEN_DEF_START
TOKEN_DEF(TEMPLATE)
TOKEN_DEF(DISABLED)
TOKEN_DEF(VISIBLE)
TOKEN_DEF(BACK)
TOKEN_DEF(IMAGE)
TOKEN_DEF(FONT_SELECTED)
TOKEN_DEF(FONT)
TOKEN_DEF(TEXT)
TOKEN_DEF(X)
TOKEN_DEF(Y)
TOKEN_DEF(WIDTH)
TOKEN_DEF(HEIGHT)
TOKEN_DEF(CURSOR_BLINK_RATE)
TOKEN_DEF(CURSOR)
TOKEN_DEF(FRAME_WIDTH)
TOKEN_DEF(NAME)
TOKEN_DEF(SCRIPT)
TOKEN_DEF(PARENT_NOTIFY)
TOKEN_DEF(MAX_LENGTH)
TOKEN_DEF(EDITOR_PROPERTY)
TOKEN_DEF(EDIT)
TOKEN_DEF(CAPTION)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////////
HRESULT CUIEdit::loadBuffer(byte *buffer, bool complete) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(TEMPLATE)
	TOKEN_TABLE(DISABLED)
	TOKEN_TABLE(VISIBLE)
	TOKEN_TABLE(BACK)
	TOKEN_TABLE(IMAGE)
	TOKEN_TABLE(FONT_SELECTED)
	TOKEN_TABLE(FONT)
	TOKEN_TABLE(TEXT)
	TOKEN_TABLE(X)
	TOKEN_TABLE(Y)
	TOKEN_TABLE(WIDTH)
	TOKEN_TABLE(HEIGHT)
	TOKEN_TABLE(CURSOR_BLINK_RATE)
	TOKEN_TABLE(CURSOR)
	TOKEN_TABLE(FRAME_WIDTH)
	TOKEN_TABLE(NAME)
	TOKEN_TABLE(SCRIPT)
	TOKEN_TABLE(PARENT_NOTIFY)
	TOKEN_TABLE(MAX_LENGTH)
	TOKEN_TABLE(EDITOR_PROPERTY)
	TOKEN_TABLE(EDIT)
	TOKEN_TABLE(CAPTION)
	TOKEN_TABLE_END

	byte *params;
	int cmd = 2;
	CBParser parser(Game);

	if (complete) {
		if (parser.GetCommand((char **)&buffer, commands, (char **)&params) != TOKEN_EDIT) {
			Game->LOG(0, "'EDIT' keyword expected.");
			return E_FAIL;
		}
		buffer = params;
	}

	while (cmd > 0 && (cmd = parser.GetCommand((char **)&buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_TEMPLATE:
			if (FAILED(loadFile((char *)params))) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_NAME:
			setName((char *)params);
			break;

		case TOKEN_BACK:
			delete _back;
			_back = new CUITiledImage(Game);
			if (!_back || FAILED(_back->loadFile((char *)params))) {
				delete _back;
				_back = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_IMAGE:
			delete _image;
			_image = new CBSprite(Game);
			if (!_image || FAILED(_image->loadFile((char *)params))) {
				delete _image;
				_image = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_FONT:
			if (_font) Game->_fontStorage->removeFont(_font);
			_font = Game->_fontStorage->addFont((char *)params);
			if (!_font) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_FONT_SELECTED:
			if (_fontSelected) Game->_fontStorage->removeFont(_fontSelected);
			_fontSelected = Game->_fontStorage->addFont((char *)params);
			if (!_fontSelected) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_TEXT:
			setText((char *)params);
			Game->_stringTable->Expand(&_text);
			break;

		case TOKEN_X:
			parser.ScanStr((char *)params, "%d", &_posX);
			break;

		case TOKEN_Y:
			parser.ScanStr((char *)params, "%d", &_posY);
			break;

		case TOKEN_WIDTH:
			parser.ScanStr((char *)params, "%d", &_width);
			break;

		case TOKEN_HEIGHT:
			parser.ScanStr((char *)params, "%d", &_height);
			break;

		case TOKEN_MAX_LENGTH:
			parser.ScanStr((char *)params, "%d", &_maxLength);
			break;

		case TOKEN_CAPTION:
			setCaption((char *)params);
			break;

		case TOKEN_CURSOR:
			delete _cursor;
			_cursor = new CBSprite(Game);
			if (!_cursor || FAILED(_cursor->loadFile((char *)params))) {
				delete _cursor;
				_cursor = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_CURSOR_BLINK_RATE:
			parser.ScanStr((char *)params, "%d", &_cursorBlinkRate);
			break;

		case TOKEN_FRAME_WIDTH:
			parser.ScanStr((char *)params, "%d", &_frameWidth);
			break;

		case TOKEN_SCRIPT:
			addScript((char *)params);
			break;

		case TOKEN_PARENT_NOTIFY:
			parser.ScanStr((char *)params, "%b", &_parentNotify);
			break;

		case TOKEN_DISABLED:
			parser.ScanStr((char *)params, "%b", &_disable);
			break;

		case TOKEN_VISIBLE:
			parser.ScanStr((char *)params, "%b", &_visible);
			break;

		case TOKEN_EDITOR_PROPERTY:
			parseEditorProperty(params, false);
			break;
		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		Game->LOG(0, "Syntax error in EDIT definition");
		return E_FAIL;
	}
	if (cmd == PARSERR_GENERIC) {
		Game->LOG(0, "Error loading EDIT definition");
		return E_FAIL;
	}

	correctSize();

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CUIEdit::saveAsText(CBDynBuffer *buffer, int indent) {
	buffer->putTextIndent(indent, "EDIT\n");
	buffer->putTextIndent(indent, "{\n");

	buffer->putTextIndent(indent + 2, "NAME=\"%s\"\n", _name);
	buffer->putTextIndent(indent + 2, "CAPTION=\"%s\"\n", getCaption());

	buffer->putTextIndent(indent + 2, "\n");

	if (_back && _back->_filename)
		buffer->putTextIndent(indent + 2, "BACK=\"%s\"\n", _back->_filename);

	if (_image && _image->_filename)
		buffer->putTextIndent(indent + 2, "IMAGE=\"%s\"\n", _image->_filename);

	if (_font && _font->_filename)
		buffer->putTextIndent(indent + 2, "FONT=\"%s\"\n", _font->_filename);
	if (_fontSelected && _fontSelected->_filename)
		buffer->putTextIndent(indent + 2, "FONT_SELECTED=\"%s\"\n", _fontSelected->_filename);

	if (_cursor && _cursor->_filename)
		buffer->putTextIndent(indent + 2, "CURSOR=\"%s\"\n", _cursor->_filename);

	buffer->putTextIndent(indent + 2, "\n");

	if (_text)
		buffer->putTextIndent(indent + 2, "TEXT=\"%s\"\n", _text);

	buffer->putTextIndent(indent + 2, "\n");

	buffer->putTextIndent(indent + 2, "X=%d\n", _posX);
	buffer->putTextIndent(indent + 2, "Y=%d\n", _posY);
	buffer->putTextIndent(indent + 2, "WIDTH=%d\n", _width);
	buffer->putTextIndent(indent + 2, "HEIGHT=%d\n", _height);
	buffer->putTextIndent(indent + 2, "MAX_LENGTH=%d\n", _maxLength);
	buffer->putTextIndent(indent + 2, "CURSOR_BLINK_RATE=%d\n", _cursorBlinkRate);
	buffer->putTextIndent(indent + 2, "FRAME_WIDTH=%d\n", _frameWidth);

	buffer->putTextIndent(indent + 2, "DISABLED=%s\n", _disable ? "TRUE" : "FALSE");
	buffer->putTextIndent(indent + 2, "VISIBLE=%s\n", _visible ? "TRUE" : "FALSE");
	buffer->putTextIndent(indent + 2, "PARENT_NOTIFY=%s\n", _parentNotify ? "TRUE" : "FALSE");

	// scripts
	for (int i = 0; i < _scripts.GetSize(); i++) {
		buffer->putTextIndent(indent + 2, "SCRIPT=\"%s\"\n", _scripts[i]->_filename);
	}

	buffer->putTextIndent(indent + 2, "\n");

	// editor properties
	CBBase::saveAsText(buffer, indent + 2);

	buffer->putTextIndent(indent, "}\n");
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
HRESULT CUIEdit::scCallMethod(CScScript *script, CScStack *stack, CScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// SetSelectedFont
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "SetSelectedFont") == 0) {
		stack->correctParams(1);

		if (_fontSelected) Game->_fontStorage->removeFont(_fontSelected);
		_fontSelected = Game->_fontStorage->addFont(stack->pop()->getString());
		stack->pushBool(_fontSelected != NULL);

		return S_OK;
	}

	else return CUIObject::scCallMethod(script, stack, thisStack, name);
}


//////////////////////////////////////////////////////////////////////////
CScValue *CUIEdit::scGetProperty(const char *name) {
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Type") == 0) {
		_scValue->setString("editor");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SelStart
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SelStart") == 0) {
		_scValue->setInt(_selStart);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SelEnd
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SelEnd") == 0) {
		_scValue->setInt(_selEnd);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// CursorBlinkRate
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "CursorBlinkRate") == 0) {
		_scValue->setInt(_cursorBlinkRate);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// CursorChar
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "CursorChar") == 0) {
		_scValue->setString(_cursorChar);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// FrameWidth
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "FrameWidth") == 0) {
		_scValue->setInt(_frameWidth);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MaxLength
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MaxLength") == 0) {
		_scValue->setInt(_maxLength);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Text
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Text") == 0) {
		if (Game->_textEncoding == TEXT_UTF8) {
			WideString wstr = StringUtil::ansiToWide(_text);
			_scValue->setString(StringUtil::wideToUtf8(wstr).c_str());
		} else {
			_scValue->setString(_text);
		}
		return _scValue;
	}

	else return CUIObject::scGetProperty(name);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIEdit::scSetProperty(const char *name, CScValue *value) {
	//////////////////////////////////////////////////////////////////////////
	// SelStart
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "SelStart") == 0) {
		_selStart = value->getInt();
		_selStart = MAX(_selStart, 0);
		_selStart = MIN((size_t)_selStart, strlen(_text));
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SelEnd
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SelEnd") == 0) {
		_selEnd = value->getInt();
		_selEnd = MAX(_selEnd, 0);
		_selEnd = MIN((size_t)_selEnd, strlen(_text));
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// CursorBlinkRate
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "CursorBlinkRate") == 0) {
		_cursorBlinkRate = value->getInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// CursorChar
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "CursorChar") == 0) {
		setCursorChar(value->getString());
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// FrameWidth
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "FrameWidth") == 0) {
		_frameWidth = value->getInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// MaxLength
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MaxLength") == 0) {
		_maxLength = value->getInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Text
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Text") == 0) {
		if (Game->_textEncoding == TEXT_UTF8) {
			WideString wstr = StringUtil::utf8ToWide(value->getString());
			setText(StringUtil::wideToAnsi(wstr).c_str());
		} else {
			setText(value->getString());
		}
		return S_OK;
	}

	else return CUIObject::scSetProperty(name, value);
}


//////////////////////////////////////////////////////////////////////////
const char *CUIEdit::scToString() {
	return "[edit]";
}


//////////////////////////////////////////////////////////////////////////
void CUIEdit::setCursorChar(const char *Char) {
	if (!Char) return;
	delete[] _cursorChar;
	_cursorChar = new char [strlen(Char) + 1];
	if (_cursorChar) strcpy(_cursorChar, Char);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIEdit::display(int OffsetX, int OffsetY) {
	if (!_visible) return S_OK;


	// hack!
	TTextEncoding OrigEncoding = Game->_textEncoding;
	Game->_textEncoding = TEXT_ANSI;

	if (_back) _back->display(OffsetX + _posX, OffsetY + _posY, _width, _height);
	if (_image) _image->Draw(OffsetX + _posX, OffsetY + _posY, NULL);

	// prepare fonts
	CBFont *font;
	CBFont *sfont;

	if (_font) font = _font;
	else font = Game->_systemFont;

	if (_fontSelected) sfont = _fontSelected;
	else sfont = font;

	bool focused = isFocused();

	_selStart = MAX(_selStart, 0);
	_selEnd   = MAX(_selEnd, 0);

	_selStart = MIN((size_t)_selStart, strlen(_text));
	_selEnd   = MIN((size_t)_selEnd,   strlen(_text));

	//int CursorWidth = font->GetCharWidth(_cursorChar[0]);
	int CursorWidth = font->getTextWidth((byte *)_cursorChar);

	int s1, s2;
	bool CurFirst;
	// modify scroll offset
	if (_selStart >= _selEnd) {
		while (font->getTextWidth((byte *)_text + _scrollOffset, MAX(0, _selEnd - _scrollOffset)) > _width - CursorWidth - 2 * _frameWidth) {
			_scrollOffset++;
			if (_scrollOffset >= (int)strlen(_text)) break;
		}

		_scrollOffset = MIN(_scrollOffset, _selEnd);

		s1 = _selEnd;
		s2 = _selStart;
		CurFirst = true;
	} else {
		while (font->getTextWidth((byte *)_text + _scrollOffset, MAX(0, _selStart - _scrollOffset)) +
		        sfont->getTextWidth((byte *)(_text + MAX(_scrollOffset, _selStart)), _selEnd - MAX(_scrollOffset, _selStart))

		        > _width - CursorWidth - 2 * _frameWidth) {
			_scrollOffset++;
			if (_scrollOffset >= (int)strlen(_text)) break;
		}

		_scrollOffset = MIN(_scrollOffset, _selEnd);

		s1 = _selStart;
		s2 = _selEnd;
		CurFirst = false;
	}


	int AlignOffset = 0;

	for (int Count = 0; Count < 2; Count++) {
		// draw text
		int xxx, yyy, width, height;

		xxx = _posX + _frameWidth + OffsetX;
		yyy = _posY + _frameWidth + OffsetY;

		width = _posX + _width + OffsetX - _frameWidth;
		height = MAX(font->getLetterHeight(), sfont->getLetterHeight());

		if (Game->_textRTL) xxx += AlignOffset;

		TTextAlign Align = TAL_LEFT;


		// unselected 1
		if (s1 > _scrollOffset) {
			if (Count) font->drawText((byte *)_text + _scrollOffset, xxx, yyy, width - xxx, Align, height, s1 - _scrollOffset);
			xxx += font->getTextWidth((byte *)_text + _scrollOffset, s1 - _scrollOffset);
			AlignOffset += font->getTextWidth((byte *)_text + _scrollOffset, s1 - _scrollOffset);
		}

		// cursor
		if (focused && CurFirst) {
			if (Count) {
				if (CBPlatform::GetTime() - _lastBlinkTime >= _cursorBlinkRate) {
					_lastBlinkTime = CBPlatform::GetTime();
					_cursorVisible = !_cursorVisible;
				}
				if (_cursorVisible)
					font->drawText((byte *)_cursorChar, xxx, yyy, width - xxx, Align, height, 1);
			}
			xxx += CursorWidth;
			AlignOffset += CursorWidth;
		}

		// selected
		int s3 = MAX(s1, _scrollOffset);

		if (s2 - s3 > 0) {
			if (Count) sfont->drawText((byte *)_text + s3, xxx, yyy, width - xxx, Align, height, s2 - s3);
			xxx += sfont->getTextWidth((byte *)_text + s3, s2 - s3);
			AlignOffset += sfont->getTextWidth((byte *)_text + s3, s2 - s3);
		}

		// cursor
		if (focused && !CurFirst) {
			if (Count) {
				if (CBPlatform::GetTime() - _lastBlinkTime >= _cursorBlinkRate) {
					_lastBlinkTime = CBPlatform::GetTime();
					_cursorVisible = !_cursorVisible;
				}
				if (_cursorVisible)
					font->drawText((byte *)_cursorChar, xxx, yyy, width - xxx, Align, height, 1);
			}
			xxx += CursorWidth;
			AlignOffset += CursorWidth;
		}

		// unselected 2
		if (Count) font->drawText((byte *)_text + s2, xxx, yyy, width - xxx, Align, height);
		AlignOffset += font->getTextWidth((byte *)_text + s2);

		AlignOffset = (_width - 2 * _frameWidth) - AlignOffset;
		if (AlignOffset < 0) AlignOffset = 0;
	}


	Game->_renderer->_rectList.Add(new CBActiveRect(Game, this, NULL, OffsetX + _posX, OffsetY + _posY, _width, _height, 100, 100, false));


	Game->_textEncoding = OrigEncoding;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
bool CUIEdit::handleKeypress(Common::Event *event, bool printable) {
	bool Handled = false;

	if (event->type == Common::EVENT_KEYDOWN && !printable) {
		switch (event->kbd.keycode) {
		case Common::KEYCODE_ESCAPE:
		case Common::KEYCODE_TAB:
		case Common::KEYCODE_RETURN:
			return false;

			// ctrl+A
		case Common::KEYCODE_a:
			if (CBKeyboardState::IsControlDown()) {
				_selStart = 0;
				_selEnd = strlen(_text);
				Handled = true;
			}
			break;

		case Common::KEYCODE_BACKSPACE:
			if (_selStart == _selEnd) {
				if (Game->_textRTL) deleteChars(_selStart, _selStart + 1);
				else deleteChars(_selStart - 1, _selStart);
			} else deleteChars(_selStart, _selEnd);
			if (_selEnd >= _selStart) _selEnd -= MAX(1, _selEnd - _selStart);
			_selStart = _selEnd;

			Handled = true;
			break;

		case Common::KEYCODE_LEFT:
		case Common::KEYCODE_UP:
			_selEnd--;
			if (!CBKeyboardState::IsShiftDown()) _selStart = _selEnd;
			Handled = true;
			break;

		case Common::KEYCODE_RIGHT:
		case Common::KEYCODE_DOWN:
			_selEnd++;
			if (!CBKeyboardState::IsShiftDown()) _selStart = _selEnd;
			Handled = true;
			break;

		case Common::KEYCODE_HOME:
			if (Game->_textRTL) {
				_selEnd = strlen(_text);
				if (!CBKeyboardState::IsShiftDown()) _selStart = _selEnd;
			} else {
				_selEnd = 0;
				if (!CBKeyboardState::IsShiftDown()) _selStart = _selEnd;
			}
			Handled = true;
			break;

		case Common::KEYCODE_END:
			if (Game->_textRTL) {
				_selEnd = 0;
				if (!CBKeyboardState::IsShiftDown()) _selStart = _selEnd;
			} else {
				_selEnd = strlen(_text);
				if (!CBKeyboardState::IsShiftDown()) _selStart = _selEnd;
			}
			Handled = true;
			break;

		case Common::KEYCODE_DELETE:
			if (_selStart == _selEnd) {
				if (Game->_textRTL) {
					deleteChars(_selStart - 1, _selStart);
					_selEnd--;
					if (_selEnd < 0) _selEnd = 0;
				} else deleteChars(_selStart, _selStart + 1);
			} else deleteChars(_selStart, _selEnd);
			if (_selEnd > _selStart) _selEnd -= (_selEnd - _selStart);

			_selStart = _selEnd;
			Handled = true;
			break;
		default:
			break;
		}
		return Handled;
	} else if (event->type == Common::EVENT_KEYDOWN && printable) {
		if (_selStart != _selEnd) deleteChars(_selStart, _selEnd);

		//WideString wstr = StringUtil::Utf8ToWide(event->kbd.ascii);
		WideString wstr;
		wstr += (char)event->kbd.ascii;
		_selEnd += insertChars(_selEnd, (byte *)StringUtil::wideToAnsi(wstr).c_str(), 1);

		if (Game->_textRTL) _selEnd = _selStart;
		else _selStart = _selEnd;

		return true;
	}

	return false;
}



//////////////////////////////////////////////////////////////////////////
int CUIEdit::deleteChars(int Start, int End) {
	if (Start > End) CBUtils::swap(&Start, &End);

	Start = MAX(Start, (int)0);
	End = MIN((size_t)End, strlen(_text));

	char *str = new char[strlen(_text) - (End - Start) + 1];
	if (str) {
		if (Start > 0) memcpy(str, _text, Start);
		memcpy(str + MAX(0, Start), _text + End, strlen(_text) - End + 1);

		delete[] _text;
		_text = str;
	}
	if (_parentNotify && _parent) _parent->applyEvent(_name);

	return End - Start;
}


//////////////////////////////////////////////////////////////////////////
int CUIEdit::insertChars(int Pos, byte *Chars, int Num) {
	if ((int)strlen(_text) + Num > _maxLength) {
		Num -= (strlen(_text) + Num - _maxLength);
	}

	Pos = MAX(Pos, (int)0);
	Pos = MIN((size_t)Pos, strlen(_text));

	char *str = new char[strlen(_text) + Num + 1];
	if (str) {
		if (Pos > 0) memcpy(str, _text, Pos);
		memcpy(str + Pos + Num, _text + Pos, strlen(_text) - Pos + 1);

		memcpy(str + Pos, Chars, Num);

		delete[] _text;
		_text = str;
	}
	if (_parentNotify && _parent) _parent->applyEvent(_name);

	return Num;
}



//////////////////////////////////////////////////////////////////////////
HRESULT CUIEdit::persist(CBPersistMgr *persistMgr) {

	CUIObject::persist(persistMgr);

	persistMgr->transfer(TMEMBER(_cursorBlinkRate));
	persistMgr->transfer(TMEMBER(_cursorChar));
	persistMgr->transfer(TMEMBER(_fontSelected));
	persistMgr->transfer(TMEMBER(_frameWidth));
	persistMgr->transfer(TMEMBER(_maxLength));
	persistMgr->transfer(TMEMBER(_scrollOffset));
	persistMgr->transfer(TMEMBER(_selEnd));
	persistMgr->transfer(TMEMBER(_selStart));

	if (!persistMgr->_saving) {
		_cursorVisible = false;
		_lastBlinkTime = 0;
	}

	return S_OK;
}

} // end of namespace WinterMute
