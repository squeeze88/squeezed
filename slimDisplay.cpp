
#include "musicDB.hpp"	//for the search menu

#include "slimDisplay.hpp"

#include "fonts/font.hpp"

// include squeezeCenter fonts:
#include "fonts/fontStandard1.h"
#include "fonts/fontStandard2.h"
#include "fonts/fontFull.h"


const font_s *fonts[] = {&fontStandard_1, &fontStandard_2, &fontFull};
static const font_s* fontPerSize[32];

//initialize the fontPerSize structure:
class fontLib_s {
public:
	fontLib_s()
	{
		//setup a default font, for all sizes:
		for(size_t i=0; i < array_size(fontPerSize); i++)
			fontPerSize[i] = &fontStandard_1;
		//override with fonts we have:
		for(size_t i=0; i < array_size(fonts); i++)
		{
			int h = fonts[i]->height;
			fontPerSize[h] = fonts[i];

		}
	}
} fontLib;


//--------------------------- Utility functions -----------------------------

const char *t9List[] = { "", " ", "abc", "def", "ghi", "jkl", "mno", "pqrs", "tuv", "wxyz"};


//test if two a falls under the T9 key 'number'
bool compareT9(char a, int number)
{
	bool isEqual = false;
	a = tolower(a);
	const char *c = t9List[number];
	for(int idx=0; c[idx] != 0; idx++)
		if( c[idx] == a )
		{
			isEqual = true;
			break;
		}
	return isEqual;
}


// text entry system for T9 keyboards, handle keypress 'key' for current string 'text'
void keyPress(char key, string *text, bool *newSymbol)
{
	int nr = key - '0';

	if( (nr >= 0) && (nr <= 9) )
	{
		const char *c = t9List[nr];

		if( *newSymbol )
		{
			(*text) += c[0];
			*newSymbol = false;
		}
		else
		{
			//check if current char in text matches any of chrList[nr],
			char *newText = &(*text)[text->size()-1];
			*newText = tolower( *newText);
			//int cursor = text->size()-1;
			bool sameT9 = false;
			int idx;
			for(idx=0; c[idx] != 0; idx++)
				if( c[idx] == *newText )
				{
					sameT9 = true;
					break;
				}
			if( sameT9 )	
				*newText = c[ (idx+1) % strlen(c) ];	// if so, goto next in chrList.
			else			
				*newText = c[0];				//else, goto first in chrList
		}
	}
}



//--------------------------- Implementation of external interface -----------


void slimDisplay::draw(char transition, int8_t param)
{
	netBuffer buf(packet);
	buf.write( (uint16_t)0 );	//offset, if ~640 for squeezebox transporter..
	buf.write( transition );	//transition ('c'=constant,'R'=right,'L'=left,'U','D')
	buf.write( (char)param );			//transition height of screen
	assert( buf.idx == 4);

	//convert uint8 to binary data:
	//char *screen = buf.data + buf.idx;
	for(int x=0; x< screenWidth; x++)
	{
		for(int y=0; y< screenHeight; y+=8)
		{
			char bin = 0;
			for(int b=0; b < 8; b++)
			{
				bin = (bin<<1) | (screen[ (y+b) * screenWidth + x]!=0);
			}
			buf.write( &bin, 1);
		}
	}

	slimConnection->send("grfe", sizeof(packet), packet);
}



void slimDisplay::putChar(char c, int fontsize, bool send)
{
	if( (size_t)fontsize >= array_size(fontPerSize) )
		return;

	const font_s *font = fontPerSize[ fontsize ];

	db_printf(15,"printing %c at %3i, %2i\t",c, pos.x, pos.y);

	//print font, have one pixel space inbetween:
	char *begin = screen + pos.x + screenWidth*pos.y;
	int width = font->render( begin, screenWidth - pos.x, screenWidth, c) + 1;

	// Update cursor:
	pos.x += width;
	/*if(pos.x > 320-8)
	{
		pos.x  = 0;
		pos.y += font->height;
	}
	if(pos.y > 32 - font->height)
		pos.y = 0;
	*/

	if(send) draw();
}



void slimDisplay::print(const char *msg, int fontSize, bool send)
{
	while( *msg != 0)
		putChar(*(msg++) , fontSize );
	if(send) draw();
}


//--------------------------- Menu system ------------------------------------


//--------------------- while playing screen -------------------

void slimPlayingMenu::draw(char transition, int8_t param)
{
	int fontSizes[] = {11, 19};

	char elapsed[10], header[80], songInfo[80];
	// Get song info:
	int elapsed_ms = display->slimConnection->status.songMSec;

	std::string group = display->slimConnection->state.currentGroup;
	const playList *list = display->slimConnection->ipc->getList( display->slimConnection->state.uuid );
	int songNr    = list->currentItem;
	int listSize  = list->items.size();
	musicFile song;
	if( listSize > 0) song =  list->items[ list->currentItem ] ;

	// Prepare strings to display:
	int secTot = elapsed_ms/1000;
	int sec = secTot % 60;
	int min = (secTot-sec) / 60;
	sprintf(elapsed, " %02i:%02i", min, sec);
	sprintf(header,  "Now Playing (%i of %i)", songNr+1, listSize);
	sprintf(songInfo,"%s (%s)", song.title.c_str(), song.artist.c_str() );

	//re-draw the entire screen:
	display->cls();
	display->gotoxy(0,0);
	display->print(header, fontSizes[0] );

	display->gotoxy( 320 - strlen(elapsed)*8  ,0);
	display->print(elapsed, fontSizes[0] );

	display->gotoxy(0, fontSizes[0] + 1 );
	display->print( songInfo, fontSizes[1] );
	display->draw(transition, param);
}


bool slimPlayingMenu::command(commands_e cmd)
{
	bool handled = true;
	switch(cmd)
	{
	case cmd_left:
		if( parent != NULL) {
			display->slimConnection->setMenu( parent , 'l');
			parent->currentItem = 0;	//scroll to our current position.
		} else
			handled = false;
		break;
	case cmd_right:
		display->draw('R');
		break;
	case cmd_playing:
		//TODO: toggle between playing screens.
		break;

	case cmd_down:
	case cmd_up:
		if( playListBrowser != NULL)
			display->slimConnection->setMenu( playListBrowser );
		//re-send the command, so the playlist browser responds to it:
		handled = playListBrowser->command(cmd);
		break;
	default:
		handled = false;
	}
	return handled;
}


//--------------------- search menu -------------------

bool slimSearchMenu::command(commands_e cmd)
{
	//on key-change: pop last query, add a new one.
	//on new key, push_back a new query, with new match.

	//on right-arrow: show list of current matches
	//on right-arrow: show list of further dbField's to sort on.

	const char *key = commandsStr[cmd];
	bool handled = false;

	char transition = 'c';

	if( (cmd==cmd_left) && (menuCursor == 0) && (match.size() < 2) && (!newSymbol) )
	{
		if( mode == textEntry)
			display->slimConnection->setMenu( parent );
		else
			mode = textEntry;
		handled = true;
	} else {
		switch(mode)
		{
		case textEntry:
			if( cmd == cmd_left )
			{
				if( !newSymbol )
					match.resize( match.size() -1);
				newSymbol = false;
			}
			else if( cmd == cmd_right )
			{
				if( newSymbol )	//search with this criterion
				{
					dbField field = query.back().getField();
					query.pop_back();			//replace the query
					query.push_back( dbQuery(db, field, match.c_str() ) );
					resultCursor = 0;
					mode = browseResults;		//show results
					transition = 'r';
				}
				else
					newSymbol = true;	//start a new character
			} else {
				keyPress( *key, &match, &newSymbol );
			}
			break;
		case fieldSelect:
			break;	//make this a sub-menu of browseResults.
		case browseResults:
			handled = true;
			if( cmd == cmd_left ) {
				mode = textEntry;
				transition = 'l';
			} else if( cmd == cmd_down ) {
				resultCursor = util::min<int>( resultCursor+1, query.back().uSize()-1 );
				transition = 'd';
			} else if( cmd == cmd_up) {
				resultCursor = util::max<int>( resultCursor-1, 0 );
				transition = 'u';
			} else if( cmd == cmd_add ) {
				std::vector<musicFile> entries = makeEntries(db, query.back(), resultCursor );
				ipc->addToGroup( display->slimConnection->currentGroup(),  entries);
			} else if( cmd == cmd_play )  {
				std::vector<musicFile> entries = makeEntries(db, query.back(), resultCursor );
				//only clear playlist if we really have something new:
				if( entries.size() > 0)
				{
					ipc->setGroup( display->slimConnection->currentGroup(),  entries);
					//and start playing:
					ipc->seekList( display->slimConnection->currentGroup(), 0, SEEK_SET );
				}
			} else {
				handled = false;
			}
			break;
		default:
			handled = false;
		} // switch (menu mode)

		if(handled)
			draw(transition,0);

	} //if (still in this menu)
	return handled;
}



void slimSearchMenu::draw(char transition, int8_t param)
{
	int fontSizes[] = {11, 19};
	display->cls();
	char title[80];
	switch(mode)
	{
	case textEntry:
		{
			sprintf(title, "text entry for %s", dbFieldStr[query.back().getField()]);

			display->gotoxy(0,0);
			display->print( title, fontSizes[0] );
			display->gotoxy(9, fontSizes[0] + 1 );
			display->print(  match.c_str(), fontSizes[1] );
			int yBot = fontSizes[0] + fontSizes[1] + 1;
			//Print cursor
			int x0 = 9 + 8*(match.size() + newSymbol);
			for(int dx = -8; dx < 0; dx++)
				display->putPixel( x0 + dx, yBot, 1);
			break;
		}
	case fieldSelect:
		break;
	case browseResults:
		{
			dbQuery *result = &query.back();
			dbField field   = result->getField();
			sprintf(title, "%s `%s*' (%lli matches)", dbFieldStr[field], match.c_str(), (LLU)result->uCount(resultCursor) );
			display->gotoxy(0,0);
			display->print( title );

			sprintf(title, "%i of %llu", resultCursor+1, (LLU)result->uSize() );
			display->gotoxy( 320 - 8*strlen(title), 0 );
			display->print( title );

			display->gotoxy(20, 8);
			//int offset = resultCursor;
			dbEntry res = (*db)[ result->uIndex(resultCursor) ];
			string val = res.getField(field);
			display->print( val.c_str() );
			break;
		}
	}
	display->draw(transition,param);
}


//--------------------- file browser menu -------------------
