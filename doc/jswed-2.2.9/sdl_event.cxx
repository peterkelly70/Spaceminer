/************************************************************************

    JSWED 2.2.7 - Editor for Jet Set Willy and derivatives

    Copyright (C) 2000-2001,2005  John Elliott <jce@seasip.demon.co.uk>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*************************************************************************/

#include "jswed.hxx"
#include "sdl_video.hxx"

static KeyListener     *gl_keyfilter;
static KeyListener     *gl_key;
static ButtonListener  *gl_button;
static MouseListener   *gl_motion;
static TickListener    *gl_tick;

static int gl_timeron = 0;
static int gl_tickraised = 0;

Uint32 gl_TimerCallback(Uint32 interval);


TickListener *setTickListener(TickListener *t)
{
	TickListener *ot = gl_tick;
	gl_tick = t;
	return ot;
}

KeyListener *setKeyListener(KeyListener *k)
{
	KeyListener *ok = gl_key;
	gl_key = k;
	return ok;
}

ButtonListener *setButtonListener(ButtonListener *b)       
{       
        ButtonListener *ob = gl_button;       
        gl_button = b;
        return ob;
}

MouseListener *setMouseListener(MouseListener *m)
{
        MouseListener *om = gl_motion;
        gl_motion = m;
        return om;
}

KeyListener *setAppKeyFilter(KeyListener *k)
{
        KeyListener *ok = gl_keyfilter;
        gl_keyfilter = k;
        return ok;
}

static int fakeKey(int keysym, bool down)
{
	int rv = -1;

	if (down)
	{
		if (gl_keyfilter) rv = gl_keyfilter->onKeyDown(keysym);
		if (rv == -1 && gl_key) rv = gl_key->onKeyDown(keysym);
	}
	else
	{
		if (gl_keyfilter) rv = gl_keyfilter->onKeyUp(keysym);
		if (rv == -1 && gl_key) rv = gl_key->onKeyUp(keysym);
	}
	return rv;
}


KeyListener    *getAppKeyFilter()	  { return gl_keyfilter; }
TickListener   *getTickListener()         { return gl_tick;      }
KeyListener    *getKeyListener()          { return gl_key;       }
ButtonListener *getButtonListener()       { return gl_button;    }
MouseListener  *getMouseListener()        { return gl_motion;    }

static int shifted(int keysym)
{
	if (keysym >= SDLK_a && keysym <= SDLK_z) return keysym - SDLK_a + 'A';

	switch(keysym)
	{
		case SDLK_BACKQUOTE:	return '|';
		case SDLK_0:		return ')';
		case SDLK_1:		return '!';
		case SDLK_2:		return '"';
                case SDLK_3:   		return ZXK_POUND;
                case SDLK_4:		return '$';
                case SDLK_5:		return '%';
                case SDLK_6:		return '^';
                case SDLK_7:		return '&';
                case SDLK_8:		return '*';
                case SDLK_9:		return '(';
		case SDLK_MINUS:	return '_';
		case SDLK_EQUALS:	return '+';
		case SDLK_LEFTBRACKET:	return '{';
		case SDLK_RIGHTBRACKET:	return '}';
		case SDLK_SEMICOLON:	return ':';
		case SDLK_QUOTE:	return '@';
		case SDLK_HASH:		return '~';
		case SDLK_BACKSLASH:	return '|';
		case SDLK_COMMA:	return '<';
		case SDLK_PERIOD:	return '>';
		case SDLK_SLASH:	return '?';
		case SDLK_SPACE:        return ZXK_SHIFTSPACE;
	}

	return keysym;
}


static int ctrled(int keysym)
{
	if (keysym >= SDLK_a && keysym <= SDLK_z) return keysym - SDLK_a + 1;

	if (keysym >= SDLK_0 && keysym <= SDLK_8) return (keysym - SDLK_1 + 128);

	return keysym;
}

static int ctrlshift(int keysym)
{
        if (keysym >= SDLK_1 && keysym <= SDLK_8) return (keysym - SDLK_1 + 136);

	return ctrled(keysym);	
}

int cursorKey(SDL_keysym *sym)
{
	int ks;

	switch(sym->sym)
	{
		case SDLK_UP:    ks = ZXK_UP;    break;
		case SDLK_DOWN:  ks = ZXK_DOWN;  break;
		case SDLK_LEFT:  ks = ZXK_LEFT;  break;
		case SDLK_RIGHT: ks = ZXK_RIGHT; break;
		default:         ks = sym->sym;  break;	
	}
	if (sym->mod & KMOD_CTRL) ks |= ZXK_CONTROL;
	if (sym->mod & KMOD_ALT)  ks |= ZXK_ALT;
	return ks;
}


int keyTrans(SDL_keysym *sym, bool down)
{
	int result = sym->sym;

	switch(result)
	{
		case SDLK_F1:  return ZXK_F1;
                case SDLK_F2:  return ZXK_F2;
                case SDLK_F3:  return ZXK_F3;
                case SDLK_F4:  return ZXK_F4;
                case SDLK_F5:  return ZXK_F5;
                case SDLK_F6:  return ZXK_F6;
                case SDLK_F7:  return ZXK_F7;
                case SDLK_F8:  return ZXK_F8;
                case SDLK_F9:  return ZXK_F9;
                case SDLK_F10: return ZXK_F10;

		case SDLK_TAB:		
				if (sym->mod & KMOD_SHIFT) return ZXK_BACKTAB;
				return ZXK_TAB;
		case SDLK_MENU:		return ZXK_EDIT;
		case SDLK_ESCAPE:	return ZXK_BREAK;
		case SDLK_RETURN:	return ZXK_ENTER;	// Return
		case SDLK_PAGEUP:	return ZXK_PGUP;	// WS page up
		case SDLK_PAGEDOWN:	return ZXK_PGDN;	// WS page down
		// Home becomes ^Q^S or ^Q^R depending
		// Similarly End becomes ^Q^D or ^Q^C
		case SDLK_HOME:	fakeKey(ZXK_CTRLQ, down);
				if (sym->mod & KMOD_SHIFT) return ZXK_PGUP;
				return ZXK_LEFT;
		case SDLK_END:	fakeKey(ZXK_CTRLQ, down);
				if (sym->mod & KMOD_SHIFT) return ZXK_PGDN;
				return ZXK_RIGHT;
		case SDLK_UP:	
		case SDLK_DOWN:
		case SDLK_LEFT:
		case SDLK_RIGHT:
					return cursorKey(sym);

		case SDLK_p:
			if (sym->mod & KMOD_ALT) return ZXK_COPY; // Copyright

		default:
			if      ((sym->mod & (KMOD_CTRL | KMOD_SHIFT)) ==
                                             (KMOD_CTRL | KMOD_SHIFT)) result = ctrlshift(result);
			else if (sym->mod & KMOD_CTRL)                result = ctrled(result);
			else if (sym->mod & KMOD_SHIFT)               result = shifted(result);

			if (sym->mod & KMOD_ALT)
			{
				result = sym->sym | ZXK_ALT;
			}
	}
	return result;
}

static int keyRepeat[SDLK_LAST];
static SDL_KeyboardEvent keyRepeatEv[SDLK_LAST];
static int gl_rpCount;	// No. of keys repeating
static int gl_timeRet;

// XXX make these user-configurable
#define REPDEL 21
#define REPPER 3

// Enable / disable key repeats.
void rpKeyDown(SDL_KeyboardEvent *ev)
{
	// CTRL/SHIFT/ALT don't repeat
	if (ev->keysym.sym == SDLK_LSHIFT || ev->keysym.sym == SDLK_RSHIFT ||
	    ev->keysym.sym == SDLK_LCTRL  || ev->keysym.sym == SDLK_RCTRL  ||
	    ev->keysym.sym == SDLK_LALT   || ev->keysym.sym == SDLK_RALT)
	{
		return;
	}
	if (keyRepeat[ev->keysym.sym] == 0)
	{
                memcpy(&keyRepeatEv[ev->keysym.sym], ev, sizeof(*ev));
        	++gl_rpCount;
        	keyRepeat[ev->keysym.sym] = -REPDEL; // REPDEL: 
	}
	else 
	{
		memcpy(&keyRepeatEv[ev->keysym.sym], ev, sizeof(*ev));
		keyRepeat[ev->keysym.sym] = REPPER;    // REPPER
	}

}

void rpKeyUp(SDL_KeyboardEvent *ev)
{
	--gl_rpCount;
	if (gl_rpCount < 0) gl_rpCount = 0;
	keyRepeat[ev->keysym.sym] = 0;	
}


Uint32 gl_TimerCallback(Uint32 interval)
{
	if (gl_sdl_locked) return interval;	// SDL screen is locked 

	gl_tickraised = 1;
	return interval;
}


void serviceTimer(void)
{
	const char *aname;
	int ainter;
	static int asvCount = 50 * 60;	// 50 jiffies * 60 secs
	static int asvInterval = 60;

	static int in_callback = 0;
	static int alerted = 0;
 
	gl_tickraised = 0;
	if (gl_tick && !in_callback)
	{
		in_callback = 1;
		gl_tick->onTick();
		in_callback = 0;
	}
	if (gl_game)
	{
		aname = getAutosaveName();
		ainter = getAutosaveInterval();
// If user has changed setting, do an autosave immediately 
		if (ainter != asvInterval) 
		{
			asvInterval = ainter;
			asvCount = 1;
		}
		if (aname[0] && ainter)
		{
			--asvCount;
			if (asvCount == 0)
			{
				asvCount = 50 * ainter;
				FILE *fp = fopen(aname, "wb");
				if (fp)
				{
					gl_game->getMem()->saveZ80(fp);
					fclose(fp);
				}
				else if (!alerted)
				{
					alert("Autosave failed", "Cancel",
			"Could not open file: %s", aname);
				}
			}
		}
	}

	if (!gl_key && !gl_keyfilter) return;
	
	// If keys are being pressed, send repeat signals for them.
	if (gl_rpCount) for (int n = SDLK_FIRST; n < SDLK_LAST; n++)
	{
		if (keyRepeat[n] == -1 || keyRepeat[n] == 1)
		{
			int rv = -1;
			int kt = keyTrans(&keyRepeatEv[n].keysym, true);

			if (gl_keyfilter) rv = gl_keyfilter->onKeyDown(kt);
			if (rv == -1 && gl_key) rv = gl_key->onKeyDown(kt);
			keyRepeat[n] = REPPER;
			// Quit > Cancel > OK > Continue. 
			if (gl_timeRet < rv) gl_timeRet = rv;	
		}
		else if (keyRepeat[n] < -1) ++keyRepeat[n];
		else if (keyRepeat[n] >  1) --keyRepeat[n];
	}
}


int videoEvent(void)
{
	int err, done = ZXE_CONTINUE;
	int t;
        SDL_Event ev;
	SDL_MouseMotionEvent *mev;
	SDL_MouseButtonEvent *bev;
	SDL_KeyboardEvent    *kev;

	gl_rpCount = 0;
	gl_timeRet = 0;
	t = gl_timeron;		// If the timer is not running, start it
	if (!t)			// It might be running if this is called
	{			// recursively.
		SDL_SetTimer(20, gl_TimerCallback);	// 50Hz clock
		gl_timeron = 1;
	}
	while (done == ZXE_CONTINUE)
	{	
		if (gl_tickraised) serviceTimer();

		// This will be set if something happened in the timer 
		// callback that demands action.
		if (gl_timeRet) done = gl_timeRet;
      
	 	err = SDL_PollEvent(&ev);

		if (err) switch(ev.type)
		{
                        case SDL_KEYDOWN:
                            kev = &ev.key;
			    rpKeyDown(kev);
                            {
	                        int r = -1;
                	        int kt = keyTrans(&kev->keysym, true);
				if (gl_keyfilter) r=gl_keyfilter->onKeyDown(kt);
				if (r== -1 && gl_key) r = gl_key->onKeyDown(kt);

				if (r != -1) done = r;
			    }
                            break;

                        case SDL_KEYUP:
                            kev = &ev.key;
			    rpKeyUp(kev);
                            {
                                int r = -1;
                                int kt = keyTrans(&kev->keysym, false);
                                if (gl_keyfilter) r=gl_keyfilter->onKeyUp(kt);
                                if (r== -1 && gl_key) r = gl_key->onKeyUp(kt);

                                if (r != -1) done = r;
                            }
			    break;

			case SDL_MOUSEBUTTONDOWN:
			    bev = &ev.button;
			    if (gl_button) 
				done = gl_button->onButtonDown(bev->x, bev->y,
                                                        bev->button);
			    break;
                        case SDL_MOUSEBUTTONUP:
                            bev = &ev.button;
                            if (gl_button)
                                done = gl_button->onButtonUp(bev->x, bev->y, 
                                                        bev->button);
                            break;
			case SDL_MOUSEMOTION:
			    mev = &ev.motion;
			    if (gl_motion)
				done = gl_motion->onMouseMove(mev->x, mev->y,
							mev->xrel, mev->yrel); 

			    break;
			case SDL_QUIT:
			    done = ZXE_QUIT; 
			    break;
		}
		else SDL_Delay(10); // Avoid CPU hogging
       	} 
	if (!t) SDL_SetTimer(0, NULL);	// If timer was not running, stop it.
	gl_timeron = t;
        return done;
}


bool shiftPressed()
{
	return (SDL_GetModState() & KMOD_SHIFT) != 0;
}


bool ctrlPressed()
{
	return (SDL_GetModState() & KMOD_CTRL) != 0;
}


bool altPressed()
{
	return (SDL_GetModState() & KMOD_ALT) != 0;
}


// Default action: Do nothing

int ButtonListener::onButtonUp  (int x, int y, int button) { return 0; }
int ButtonListener::onButtonDown(int x, int y, int button) { return 0; }
int KeyListener   ::onKeyUp     (int keysym)               { return 0; }
int KeyListener   ::onKeyDown   (int keysym)               { return 0; }
int MouseListener ::onMouseMove (int x, int y, int xrel, int yrel) { return 0; }
void TickListener::onTick() {}

