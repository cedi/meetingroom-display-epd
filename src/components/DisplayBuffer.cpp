#include "components/components.h"

// Function to test if a specific alignment flag is set
bool hasAlignment(uint8_t alignment, Alignment flag)
{
	return (alignment & flag) == flag;
}

void DisplayBuffer::firstPage()
{
	display->setRotation(0);
	display->setTextWrap(false);

	setTextSize(1);
	display->setFullWindow();
	setForegroundColor(Color::Black);
	setBackgroundColor(Color::White);

	clearDisplay();
}

void DisplayBuffer::clearDisplay()
{
	display->setFullWindow();
	display->fillScreen(backgroundColor);
}

// Draw a String on x/y coordinate
void DisplayBuffer::drawString(int16_t x, int16_t y, const String &text, uint8_t alignment)
{
	int16_t x1, y1;
	uint16_t w, h;

	display->setTextColor(foregroundColor);
	display->getTextBounds(text, x, y, &x1, &y1, &w, &h);

	if (hasAlignment(alignment, Alignment::HorizontalCenter))
	{
		x = x - w / 2;
	}
	else if (hasAlignment(alignment, Alignment::Right))
	{
		x = x - w;
	}
	else if (hasAlignment(alignment, Alignment::Left))
	{
		x = x;
	}

	if (hasAlignment(alignment, Alignment::VerticalCenter))
	{
		y = y + h / 2;
	}
	else if (hasAlignment(alignment, Alignment::Top))
	{
		y = y + h;
	}
	else if (hasAlignment(alignment, Alignment::Bottom))
	{
		y = y;
	}

	display->setCursor(x, y);
	display->print(text);
}

TextSize *DisplayBuffer::getStringBounds(const String &text)
{
	int16_t x1, y1;
	uint16_t w, h;
	display->getTextBounds(text, 0, 0, &x1, &y1, &w, &h);

	TextSize *size = new TextSize();
	size->width = w;
	size->height = h;
	return size;
}

TextSize *DisplayBuffer::getStringBounds(const String &text, uint16_t max_width, uint16_t max_lines)
{
	Serial.printf("text: %s\n", text.c_str());

	uint16_t current_line = 0;
	String textRemaining = text;

	TextSize *biggestTextSize = new TextSize();
	biggestTextSize->width = 0;
	biggestTextSize->height = 0;

	// print until we reach max_lines or no more text remains
	while (current_line < max_lines && !textRemaining.isEmpty())
	{
		Serial.printf("current_line: %d, textRemaining: %s\n", current_line, textRemaining.c_str());

		int16_t x1, y1;
		uint16_t w, h;

		display->getTextBounds(textRemaining, 0, 0, &x1, &y1, &w, &h);
		int16_t line_spacing = h;

		if(w > biggestTextSize->width)
		{
			biggestTextSize->width = w;
		}

		biggestTextSize->height += h;

		Serial.printf("current_line: %d, line_spacing: %d, max_height: %d\n", current_line, line_spacing, biggestTextSize->height);

		int endIndex = textRemaining.length();
		// check if remaining text is to wide, if it is then print what we can
		String subStr = textRemaining;
		int splitAt = 0;
		int keepLastChar = 0;
		while (w > max_width && splitAt != -1)
		{
			if (keepLastChar)
			{
				// if we kept the last character during the last iteration of this while
				// loop, remove it now so we don't get stuck in an infinite loop.
				subStr.remove(subStr.length() - 1);
			}

			// find the last place in the string that we can break it.
			if (current_line < max_lines - 1)
			{
				splitAt = std::max(subStr.lastIndexOf(" "),
								   subStr.lastIndexOf("-"));
			}
			else
			{
				// this is the last line, only break at spaces so we can add ellipsis
				splitAt = subStr.lastIndexOf(" ");
			}

			// if splitAt == -1 then there is an unbroken set of characters that is
			// longer than max_width. Otherwise if splitAt != -1 then we can continue
			// the loop until the string is <= max_width
			if (splitAt != -1)
			{
				endIndex = splitAt;
				subStr = subStr.substring(0, endIndex + 1);

				char lastChar = subStr.charAt(endIndex);
				if (lastChar == ' ')
				{
					// remove this char now so it is not counted towards line width
					keepLastChar = 0;
					subStr.remove(endIndex);
					--endIndex;
				}
				else if (lastChar == '-')
				{
					// this char will be printed on this line and removed next iteration
					keepLastChar = 1;
				}

				if (current_line < max_lines - 1)
				{
					// this is not the last line
					display->getTextBounds(subStr, 0, 0, &x1, &y1, &w, &h);
				}
				else
				{
					// this is the last line, we need to make sure there is space for
					// ellipsis
					display->getTextBounds(subStr + "...", 0, 0, &x1, &y1, &w, &h);
					if (w <= max_width)
					{
						// ellipsis fit, add them to subStr
						subStr = subStr + "...";
					}
				}

			}
		}

		Serial.printf("current_line: %d, line_spacing: %d, max_height: %d, substr: %s\n", current_line, line_spacing, biggestTextSize->height, subStr.c_str());

		// update textRemaining to no longer include what was printed
		// +1 for exclusive bounds, +1 to get passed space/dash
		textRemaining = textRemaining.substring(endIndex + 2 - keepLastChar);

		Serial.printf("current_line: %d, line_spacing: %d, max_height: %d, textRemaining: %s\n", current_line, line_spacing, biggestTextSize->height, textRemaining.c_str());

		++current_line;
	}

	return biggestTextSize;
}


void DisplayBuffer::drawBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t width, int16_t height)
{
	display->drawInvertedBitmap(x, y, bitmap, width, height, foregroundColor);
}

void DisplayBuffer::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t thickness)
{
	display->fillRect(x, y, thickness, h, foregroundColor);					// left column
	display->fillRect(x + w - thickness, y, thickness, h, foregroundColor); // right column

	display->fillRect(x, y, w, thickness, foregroundColor);					// top column
	display->fillRect(x, y + h - thickness, w, thickness, foregroundColor); // bottom column
}

/* Draws a string that will flow into the next line when max_width is reached.
 * If a string exceeds max_lines an ellipsis (...) will terminate the last word.
 * Lines will break at spaces(' ') and dashes('-').
 *
 * Note: max_width should be big enough to accommodate the largest word that
 *       will be displayed. If an unbroken string of characters longer than
 *       max_width exist in text, then the string will be printed beyond
 *       max_width.
 */
void DisplayBuffer::drawString(int16_t x, int16_t y, const String &text, uint8_t alignment, uint16_t max_width, uint16_t max_lines)
{
	uint16_t current_line = 0;
	String textRemaining = text;

	// print until we reach max_lines or no more text remains
	while (current_line < max_lines && !textRemaining.isEmpty())
	{
		int16_t x1, y1;
		uint16_t w, h;

		display->getTextBounds(textRemaining, 0, 0, &x1, &y1, &w, &h);
		int16_t line_spacing = h;

		int endIndex = textRemaining.length();
		// check if remaining text is to wide, if it is then print what we can
		String subStr = textRemaining;
		int splitAt = 0;
		int keepLastChar = 0;
		while (w > max_width && splitAt != -1)
		{
			if (keepLastChar)
			{
				// if we kept the last character during the last iteration of this while
				// loop, remove it now so we don't get stuck in an infinite loop.
				subStr.remove(subStr.length() - 1);
			}

			// find the last place in the string that we can break it.
			if (current_line < max_lines - 1)
			{
				splitAt = std::max(subStr.lastIndexOf(" "),
								   subStr.lastIndexOf("-"));
			}
			else
			{
				// this is the last line, only break at spaces so we can add ellipsis
				splitAt = subStr.lastIndexOf(" ");
			}

			// if splitAt == -1 then there is an unbroken set of characters that is
			// longer than max_width. Otherwise if splitAt != -1 then we can continue
			// the loop until the string is <= max_width
			if (splitAt != -1)
			{
				endIndex = splitAt;
				subStr = subStr.substring(0, endIndex + 1);

				char lastChar = subStr.charAt(endIndex);
				if (lastChar == ' ')
				{
					// remove this char now so it is not counted towards line width
					keepLastChar = 0;
					subStr.remove(endIndex);
					--endIndex;
				}
				else if (lastChar == '-')
				{
					// this char will be printed on this line and removed next iteration
					keepLastChar = 1;
				}

				if (current_line < max_lines - 1)
				{
					// this is not the last line
					display->getTextBounds(subStr, 0, 0, &x1, &y1, &w, &h);
				}
				else
				{
					// this is the last line, we need to make sure there is space for
					// ellipsis
					display->getTextBounds(subStr + "...", 0, 0, &x1, &y1, &w, &h);
					if (w <= max_width)
					{
						// ellipsis fit, add them to subStr
						subStr = subStr + "...";
					}
				}

			}
		}

		drawString(x, y + (current_line * line_spacing), subStr, alignment);

		// update textRemaining to no longer include what was printed
		// +1 for exclusive bounds, +1 to get passed space/dash
		textRemaining = textRemaining.substring(endIndex + 2 - keepLastChar);

		++current_line;
	}
}