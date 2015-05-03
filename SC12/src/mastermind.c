/*
    The MIT License (MIT)

    Copyright (c) 2015 Dries007

    Made for/in relation to an education at Thomas More Mechelen-Antwerpen vzw
    Campus De Nayer - Professionele bachelor elektronica-ict

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
 */

#include "mastermind.h"

/*************************************************************
 *                      USER RELATED
 *************************************************************/

User * getUserByIP(long far * ip)
{
    struct userlist_el * current = listHead;
    while (current != NULL)
    {
        if (current->user.ip == *((unsigned long *) ip)) return &(current->user);
        current = current->next;
    }
    return NULL;
}

User * getUserByName(char * name)
{
    for(int i = 0; name[i]; i++) name[i] = tolower(name[i]);

    struct userlist_el * current = listHead;
    while (current != NULL)
    {
        if (strcmp(current->user.name, name) == 0) return &(current->user);
        current = current->next;
    }
    return NULL;
}

void addUser(long ip, char name[21])
{
    for(int i = 0; name[i]; i++) name[i] = tolower(name[i]);
    struct userlist_el * newItem = (struct userlist_el *) malloc(sizeof(struct userlist_el));

    newItem->user.ip = ip;
    strcpy(newItem->user.name, name);

    if (listHead == NULL)
    {
        listHead = listTail = newItem;
    }
    else
    {
        listTail->next = newItem;
    }
}

void printAllUsers()
{
    if (listHead == NULL)
    {
        printf("No users in the user list.\n");
        return;
    }

    printf("User list:\n");
    struct userlist_el * current = listHead;
    while (current != NULL)
    {
        printf("Username: %s Remote IP: %d.%d.%d.%d\n", current->user.name, (int)((current->user.ip & 0xFF000000l) >> 24), (int)((current->user.ip & 0x00FF0000l) >> 16), (int)((current->user.ip & 0x0000FF00l) >> 8 ), (int)(current->user.ip & 0x000000FFl));
        current = current->next;
    }
}

/*************************************************************
 *                      LEDS RELATED
 *************************************************************/

void sendLEDS()
{
    address addr = RAM_LEDS_START;
    for (byte r = 0; r < ROWS; r++)
    {
        for (byte p = 0; p < COLORS; p++)
        {
            RGB rgb = ALL_COLORS[game.guesses[r][p]];
            writeDatabus(addr++, rgb.r);
            writeDatabus(addr++, rgb.g);
            writeDatabus(addr++, rgb.b);
        }
        byte g = game.guesses[r][COLORS];
        byte r = game.guesses[r][COLORS + 1];

        if (g + r > COLORS) printf("BUG!! r + g > COLORS with r = %d; g = %d; COLORS = %d\n", r, g, COLORS);

        for (byte p = 0; p < g; p++)
        {
            writeDatabus(addr++, GREEN.r);
            writeDatabus(addr++, GREEN.g);
            writeDatabus(addr++, GREEN.b);
        }

        for (byte p = 0; p < r; p++)
        {
            writeDatabus(addr++, ORANGE.r);
            writeDatabus(addr++, ORANGE.g);
            writeDatabus(addr++, ORANGE.b);
        }

        for (byte p = g + r; p < COLORS; p++)
        {
            writeDatabus(addr++, BLACK.r);
            writeDatabus(addr++, BLACK.g);
            writeDatabus(addr++, BLACK.b);
        }
    }
    writeDatabus(RAM_INT_SEND, CMD_LEDS_SEND);
}

/*************************************************************
 *                      GAME RELATED
 *************************************************************/

Game * getGame()
{
    return &game;
}

void resetGame()
{
    game.state = STATE_NO_GAME;
    game.nrOfGuesses = 0;
    for (int i = 0; i < ROWS; i++)
    {
        for (int j = 0; j < COLORS + 2; j++)
        {
            game.guesses[i][j] = 9; // 9 = black
        }
    }
    sendLEDS();
}

void setRndCode(byte colors)
{
    if (colors > MAX_COLORS) colors = MAX_COLORS;
    for (byte i = 0; i < COLORS; i++)
    {
        game.code[i] = rand() % colors;
    }
}

void guessRow(byte id)
{
    for (byte i = 0; i < COLORS; i++)
    {
        if (game.guesses[id][i] == game.code[i])
        {
            game.guesses[id][COLORS] ++;
        }
        else
        {
             // + 1 because we know i doesn't match
            for (byte j = i + 1; j < COLORS; j++)
            {
                if (game.guesses[id][i] == game.code[j])
                {
                    game.guesses[id][COLORS + 1] ++;
                    break;
                }
            }
        }
    }
    sendLEDS();
}

/*************************************************************
 *                      DATA BUS RELATED
 *************************************************************/

void enableDatabus()
{
    pfe_enable_bus(0xFF, 1);    // all 8 bits enabled with ALE
    pfe_enable_pcs(1);          // Chip select 1 (PIO4)
    pfe_enable_pio(2, 5);       // PIO2 = output, low
    pfe_enable_pio(3, 5);       // PIO3 = output, low
}

byte readDatabus(address addr)
{
    byte bank = addr >> 8;
    pfe_enable_pio(2, bank & 0x01 ? 4 : 5); // if bank is 1 or 3, set PIO2 high, otherwise set PIO2 low
    pfe_enable_pio(3, bank & 0x02 ? 4 : 5); // if bank is 3 or 4, set PIO3 high, otherwise set PIO3 low
    return hal_read_bus((addr & 0x0FF) | 0x100, 0xFFFF, 0x0000); // Read data bus on corrected address (always in 0x100..0x1FF range)
}

void writeDatabus(address addr, byte val)
{
    byte bank = addr >> 8;
    pfe_enable_pio(2, bank & 0x01 ? 4 : 5); // if bank is 1 or 3, set PIO2 high, otherwise set PIO2 low
    pfe_enable_pio(3, bank & 0x02 ? 4 : 5); // if bank is 3 or 4, set PIO3 high, otherwise set PIO3 low
    hal_write_bus((addr & 0x0FF) | 0x100, val, 0xFFFF, 0x0000); // Write to data bus on corrected address (always in 0x100..0x1FF range)
}

/*************************************************************
 *                      LCD RELATED
 *************************************************************/

void clearLCD()
{
    for (address addr = RAM_LCD_START; addr <= RAM_LCD_END; addr++)
    {
        writeDatabus(addr, ' ');
    }
    writeDatabus(RAM_LCD_CMD, 0x01);
    writeDatabus(RAM_INT_SEND, CMD_LCD_CMD);
}

void setLCDLine(byte line, const char * text)
{
    address offset = RAM_LCD_START;

#if LCD_LINES == 2
    if (line == 1 || line == 3) offset += 40;
#elif LCD_LINES == 4
    if (line % 2 != 0) offset += 40;
    if (line > 1) offset += 0x14;
#endif

    address i = 0;
    for (; i < strlen(text); i++) // chars
    {
        writeDatabus(i + offset, text[i]);
    }
    for (; i < LCD_LINE_SIZE; i++) // spaces
    {
        writeDatabus(i + offset, ' ');
    }
    writeDatabus(RAM_INT_SEND, CMD_LCD_CL_PR);
    delay(100);
}

void setLCDLineFormat(byte line, const char * format, ...)
{
    byte buffer[LCD_LINE_SIZE + 1];
    buffer[LCD_LINE_SIZE] = 0x00;

    /* Start magic */
    va_list aptr;
    va_start(aptr, format);
    vsprintf(buffer, format, aptr);
    va_end(aptr);
    /* End magic */

    setLCDLine(line, buffer);
}

/****************************************************************************************************
 *                                              TASKS
 ****************************************************************************************************/

byte LCDupdateRunning;

void LCDupdate()
{
    LCDupdateRunning = 1;
    byte ip[16], oldIp[16];

    while (LCDupdateRunning)
    {
        Get_IPConfig(ip, NULL, NULL);

        if (strcmp(ip, oldIp) != 0)
        {
            strcpy(oldIp, ip);
            setLCDLine(1, ip);
        }
        RTX_Sleep_Time(2500);
    }
}

unsigned int LCDUpdate_stack[TASK_STACKSIZE/sizeof(unsigned int)];
int LCDupdateID;

TaskDefBlock LCDupdateTaskDefBlock =
{
    LCDupdate,
    {'L', 'C', 'D', ' '},
    &LCDUpdate_stack[TASK_STACKSIZE/sizeof(unsigned int)],  // top of stack
    TASK_STACKSIZE,                                         // size of stack
    0,                                                      // attributes, not supported
    100,                                                    // lower priority than any system tasks
    0,                                                      // time slice (if any), not supported
    0,0,0,0                                                 // mailboxes
};

/****************************************************************************************************
 *                                              MAIN
 ****************************************************************************************************/

/*
 * Ends all tasks, returns focus and exits
 */
void endProgram()
{
    setLCDLine(0, "Shutdown issued");

    removeCGIMethods();

    printf("Gracefully ending all tasks...\n");

    LCDupdateRunning = 0; // tell tasks to stop

    RTX_Sleep_Time(3000); // give tasks time to end

    printf("Killing the non obedient tasks...\n");
    RTX_Delete_Task(LCDupdateID);

    printf("\nEND OF PROGRAM\n");
    // Release input/output
    BIOS_Set_Focus(FOCUS_BOTH);
    exit(0);
}

void test()
{
    writeDatabus(RAM_LEDS_DIM, 0xFF);
    writeDatabus(RAM_LEDS_AMOUNT, 30);

    address addr = RAM_LEDS_START;
    for (byte r = 0; r < ROWS; r++)
    {
        for (byte p = 0; p < COLORS; p++)
        {
            RGB rgb = ALL_COLORS[rand() % MAX_COLORS];
            writeDatabus(addr++, rgb.r);
            writeDatabus(addr++, rgb.g);
            writeDatabus(addr++, rgb.b);
        }

        byte g = 1;
        byte r = 1;

        if (g + r > COLORS) printf("BUG!! r + g > COLORS with r = %d; g = %d; COLORS = %d\n", r, g, COLORS);

        for (byte p = 0; p < g; p++)
        {
            writeDatabus(addr++, GREEN.r);
            writeDatabus(addr++, GREEN.g);
            writeDatabus(addr++, GREEN.b);
        }

        for (byte p = 0; p < r; p++)
        {
            writeDatabus(addr++, ORANGE.r);
            writeDatabus(addr++, ORANGE.g);
            writeDatabus(addr++, ORANGE.b);
        }

        for (byte p = g + r; p < COLORS; p++)
        {
            writeDatabus(addr++, BLACK.r);
            writeDatabus(addr++, BLACK.g);
            writeDatabus(addr++, BLACK.b);
        }
    }
    writeDatabus(RAM_INT_SEND, CMD_LEDS_SEND);
    delay(1000);

    /*
    byte buffer[] = "TEST LCD";
    for (address i = 0; i < strlen(buffer) + 1; i++)
    {
       writeDatabus(i + RAM_LCD_START, buffer[i]);
    }
    writeDatabus(RAM_INT_SEND, CMD_LCD_CHAR);
    */

    setLCDLine(0, "TEST Line 0"); delay(100);
    setLCDLine(1, "TEST Line 1"); delay(100);
    setLCDLine(2, "TEST Line 2"); delay(100);
    setLCDLine(3, "TEST Line 3"); delay(100);
}

byte intByte = 0x00;

void huge _pascal _saveregs int0handler() // Declare this function as an ISR!
{
    intByte = readDatabus(RAM_INT_GET); // read location 1fe and reset int condition
    // Give specific int0 EOI to the PIC (SC12 i/o base address ff00h) see 80186 USERS MAN.
    //outport (0xff22, INTR0);
}

void main()
{
    // Get focus
    BIOS_Set_Focus(FOCUS_APPLICATION);

    /**
     * INIT All of the things!
     */
    enableDatabus();

    writeDatabus(RAM_INT_SEND, CMD_LCD_BL_ON);

    writeDatabus(RAM_LCD_CMD, 0x0C); // Blink off
    writeDatabus(RAM_INT_SEND, CMD_LCD_CMD);

    clearLCD();

    printf("\n\nAVR firmware version id: %d 0x%02x\n\n\n", readDatabus(RAM_VERSION_1), readDatabus(RAM_VERSION_2));
    setLCDLine(0, "MMM by Dries007");
    setLCDLine(1, "Booting...");

    // Setup AVR for LCD
    //initLcd();

    // Print firmware info to LCD
    //setLCDLineFormat(0, "(C) Dries007 '%s", CYEAR);
    //setLCDLineFormat(1, "Version %s.%d", VERSION, readDatabus(ADDRESS_FW_VER));

    // Ethernet connection check
    if (BIOS_Ethernet_State(NULL, NULL))
    {
        setLCDLine(0, "ERROR");
        setLCDLine(1, "NO ETHERNET!");
        return;
    }

    /**
     * RUN ALL TASKS
     */
    int result = RTX_Create_Task(&LCDupdateID , &LCDupdateTaskDefBlock);

    if (result!=0)
    {
        printf("Creating/restart LCDupdate failed %d, exit program\n", result);
        //delete task1
        RTX_Delete_Task(LCDupdateID);
        endProgram();
    }

    // Do time setup
    // printf("Fetching time...\n");
    // initTime();

    /**
     * CGI methods
     */

    printf("Installing CGI methods\n");
    installCGIMethods();

    //pfe_enable_int(0);
    //pfe_set_edge_level_intr_mode(0, 0);
    //hal_install_isr(0, 1, &int0handler);

    /*
     * MENU
     */
    byte key;
    while (1)
    {
        printf("-~= Menu =~-\n");
        printf("------------\n");
        printf("[X] End program\n");
        printf("[R] Reboot\n");
        printf("[D] Debug RAM Dump\n");
        printf("[S] Set RAM manually\n");
        // printf("[T] Redo time update\n");
        printf("[U] Print all known users\n");
        printf("[I] Interrupt test to AVR\n");

        scanf("%c%*c", &key);

        printf("IntByte: 0x%02x Real: 0x%02x\n", intByte, readDatabus(RAM_INT_GET));

        switch (key & ~0x20)
        {
        case 'X':
            endProgram();
            break;
        case 'R':
            BIOS_Reboot();
            break;
        case 'D':
            ramdump();
            break;
        case 'S':
            manualram();
            break;
//        case 'T':
//            initTime();
//            break;
        case 'U':
            printAllUsers();
            break;
        case 'I':
            test();
            break;
        default:
            printf("Char not in menu: %c\n", key);
        }
    }
}

/****************************************************************************************************
 *                                              WEB
 ****************************************************************************************************/

/*************************************************************
 *                      TEMPLATE PARTS
 *************************************************************/

char * pageHeader = "<!doctype html><html><head><meta charset='us-ascii'/><meta name='viewport' content='width=400' /><title>Mastermind</title><link href='http://fonts.googleapis.com/css?family=Open+Sans:600,400' rel='stylesheet' type='text/css'><link href='style.css' rel='stylesheet'/></head><body><div id='wrapper'>";
char * pageFooter = "</div><footer><a class='center' href='http://www.dries007.net/'>&copy; Dries007.net - 2015</a></footer></body></html>";
char * pickUsername = "<h2>Welcome new player!</h2><p>Before you can play, you need to pick a username:</p><form method='post' action='pickUsername'><input type='text' name='username' maxlength='20'/><input type='submit' value='Check'/></form>";
char * noGameYet = "<p>No game is going yet, but you can start one!</p>";
char * gameAvailable = "<p>A game is currently being played. Go ahead an join!</p>";

/**
 * Needs 2 extra strings per count. First string is URL, second is name.
 */
void addMenuItems(char * buffer, byte count, ...)
{
    strcat(buffer, "<header class='center'><h1>Mastermind</h1><ul>");

    va_list ap;
    va_start(ap, count);

    for (byte i = 0; i < count; i++)
    {
        strcat(buffer, "<li><a href='");
        strcat(buffer, va_arg(ap, char *));
        strcat(buffer, "'>");
        strcat(buffer, va_arg(ap, char *));
        strcat(buffer, "</a></li>");
    }

    va_end(ap);
    strcat(buffer, "</ul></header>");
}


/*************************************************************
 *                      HOME (GET)
 *************************************************************/

void huge _pascal _saveregs cgiHomeFunction(rpCgiPtr CgiRequest)
{
    static char pageBuffer[2048];  // Buffer to contain web page
    //char tmpBuffer[512]; // Buffer for string manipulation functions

    sprintf(pageBuffer, pageHeader);

    User * user = getUserByIP(CgiRequest->fRemoteIPPtr);
    if (user == NULL)
    {
        addMenuItems(pageBuffer, 0);
        strcat(pageBuffer, pickUsername);
    }
    else
    {
        if (getGame()->state == STATE_NO_GAME) addMenuItems(pageBuffer, 1, "start", "Start a game");
        else addMenuItems(pageBuffer, 1, "play", "Play");

        strcat(pageBuffer, "<h2>Welcome ");
        strcat(pageBuffer, user->name);
        strcat(pageBuffer, "</h2>");

        if (getGame()->state == STATE_NO_GAME) strcat(pageBuffer, noGameYet);
        else strcat(pageBuffer, gameAvailable);
    }

    strcat(pageBuffer, pageFooter);

    CgiRequest->fHttpResponse = CgiHttpOk;
    CgiRequest->fDataType = CGIDataTypeHtml;
    CgiRequest->fResponseBufferPtr = pageBuffer;
    CgiRequest->fResponseBufferLength = strlen(pageBuffer);
}

/*************************************************************
 *                      PICK USERNAME (POST)
 *************************************************************/

void huge _pascal _saveregs cgiPickUsernameFunction(rpCgiPtr CgiRequest)
{
    static char pageBuffer[2048];  // Buffer to contain web page
    //char tmpBuffer[512]; // Buffer for string manipulation functions

    sprintf(pageBuffer, pageHeader);

    char * name;
    char * value;
    while (CGI_GetArgument(&name, &value, CgiRequest) == CGI_ARGUMENT_ERR_OK)
    {
        if (strcmp(name, "username") == 0)
        {
            if (getUserByName(value) != NULL)
            {
                addMenuItems(pageBuffer, 0);

                strcat(pageBuffer, "<p>Sorry, ");
                strcat(pageBuffer, value);
                strcat(pageBuffer, " is already in use. Pick another name please:</p><form method='post' action='pickUsername'><input type='text' name='username'/><input type='submit' value='Check'/></form>");
            }
            else
            {
                addUser(*((long *)CgiRequest->fRemoteIPPtr), value);

                if (getGame()->state == STATE_NO_GAME) addMenuItems(pageBuffer, 1, "start", "Start a game");
                else addMenuItems(pageBuffer, 1, "play", "Play");

                strcat(pageBuffer, "<p>You are now known as ");
                strcat(pageBuffer, value);
                strcat(pageBuffer, "!</p>");

                if (getGame()->state == STATE_NO_GAME) strcat(pageBuffer, noGameYet);
                else strcat(pageBuffer, gameAvailable);
            }
        }
    }

    strcat(pageBuffer, pageFooter);

    CgiRequest->fHttpResponse = CgiHttpOk;
    CgiRequest->fDataType = CGIDataTypeHtml;
    CgiRequest->fResponseBufferPtr = pageBuffer;
    CgiRequest->fResponseBufferLength = strlen(pageBuffer);
}


/*************************************************************
 *                      START (BOTH)
 *************************************************************/

void huge _pascal _saveregs cgiStartFunction(rpCgiPtr CgiRequest)
{
    static char pageBuffer[2048];  // Buffer to contain web page
    char tmpBuffer[512]; // Buffer for string manipulation functions

    sprintf(pageBuffer, pageHeader);

    User * user = getUserByIP(CgiRequest->fRemoteIPPtr);
    Game * game = getGame();
    if (user == NULL)
    {
        addMenuItems(pageBuffer, 0);
        strcat(pageBuffer, pickUsername);
    }
    else if (game->state == STATE_NO_GAME || game->state == STATE_GAME_CONFIGURED) // If game is not (fully) configured yet
    {
        char * name;
        char * value;
        while (CGI_GetArgument(&name, &value, CgiRequest) == CGI_ARGUMENT_ERR_OK) // Argument parse loop
        {
            if (strcmp(name, "mode") == 0) // Gamemode
            {
                game->vsPlayer = strcmp(value, "Player") == 0;
                game->state = STATE_GAME_CONFIGURED;
            }
            else if (strcmp(name, "colors") == 0) // # of colors
            {
                game->colors = atoi(value);
                game->state = STATE_GAME_CONFIGURED;
            }
            else // Colors of code
            {
                int i;
                sscanf(name, "c%d", &i);
                game->code[i] = atoi(value);
                game->state = STATE_GAME_STARTED;
            }
        }

        if (game->state == STATE_GAME_CONFIGURED) // If game is partially configured (argument parser above)
        {
            if (game->vsPlayer) // Print color picker code
            {
                game->host = user;
                addMenuItems(pageBuffer, 0);
                strcat(pageBuffer, "<p>Pick your code:</p><form method='get' action='start'>");
                for (byte i = 0; i < 4; i++) // Color picker 1 -> 4
                {
                    sprintf(tmpBuffer, "<select name='c%d'>", i);
                    strcat(pageBuffer, tmpBuffer);

                    for (byte c = 0; c < game->colors; c++)
                    {
                        sprintf(tmpBuffer, "<option value='%d' class='%s'>%s</option>", c, ALL_COLOR_CLASSES[c], ALL_COLOR_CLASSES[c]);
                        strcat(pageBuffer, tmpBuffer);
                    }

                    strcat(pageBuffer, "</select>");
                }
                strcat(pageBuffer, "<input type='submit' value='Choose!'/></form>");
            }
            else // VS computer
            {
                addMenuItems(pageBuffer, 1, "play", "Play");
                strcat(pageBuffer, "<h2>Game started!</h2>");
                setRndCode(game->colors);
                game->state = STATE_GAME_STARTED;
            }
        }
        else if (game->state == STATE_GAME_STARTED) // Game started
        {
            addMenuItems(pageBuffer, 1, "play", "Play");
            strcat(pageBuffer, "<h2>Game started!</h2>");
        }
        else // New game form
        {
            addMenuItems(pageBuffer, 0);
            strcat(pageBuffer, "<h2>Start a new game</h2><form method='get' action='start'><p> Player(s) VS <label><input type='radio' name='mode' value='Player' checked/> Host</label><label><input type='radio' name='mode' value='Computer'/> Computer</label></p><p><label for='colors'># of colors: </label><select id='colors' name='colors'><option>4</option><option>6</option><option>8</option><option>10</option></select></p><input type='submit' value='Go!'/></form>");
        }
    }
    else // Already going
    {
        addMenuItems(pageBuffer, 1, "play", "Play");
        strcat(pageBuffer, "<h2>Start a new game</h2><p>A game has already been started.</p>");
    }

    strcat(pageBuffer, pageFooter);

    CgiRequest->fHttpResponse = CgiHttpOk;
    CgiRequest->fDataType = CGIDataTypeHtml;
    CgiRequest->fResponseBufferPtr = pageBuffer;
    CgiRequest->fResponseBufferLength = strlen(pageBuffer);
}

/*************************************************************
 *                      PLAY (GET)
 *************************************************************/

void huge _pascal _saveregs cgiPlayFunction(rpCgiPtr CgiRequest)
{
    static char pageBuffer[2048];  // Buffer to contain web page
    char tmpBuffer[512]; // Buffer for string manipulation functions

    sprintf(pageBuffer, pageHeader);

    User * user = getUserByIP(CgiRequest->fRemoteIPPtr);
    Game * game = getGame();

    if (user == NULL)
    {
        addMenuItems(pageBuffer, 0);
        strcat(pageBuffer, pickUsername);
    }
    else if (game->state == STATE_NO_GAME) // If game is not configured yet
    {
        addMenuItems(pageBuffer, 1, "start", "Start a game");
        strcat(pageBuffer, noGameYet);
    }
    else
    {
        addMenuItems(pageBuffer, 0);

        // Process guess, if any
        char * name;
        char * value;
        byte i = 0xFF;
        while (CGI_GetArgument(&name, &value, CgiRequest) == CGI_ARGUMENT_ERR_OK)
        {
            int p, c; //p = position, c = color id
            sscanf(name, "c%d", &p);
            c = atoi(value);
            if (i == 0xFF) i = game->nrOfGuesses++;
            game->guesses[i][p] = c;
        }
        if (i != 0xFF) guessRow(i);

        // Display guess table

        strcat(pageBuffer, "<p>Guesses:</p><table class='guesses' border> <tr> <th class='txt'>#</th>");
        for (byte c = 0; c < COLORS; c++) strcat(pageBuffer, "<th style='min-width: 50px;'></th>");
        strcat(pageBuffer, "<th class='txt'>Exact</th> <th class='txt'>Color</th> </tr>");

        for (byte i = 0; i < game->nrOfGuesses; i++)
        {
            sprintf(tmpBuffer, "<tr><td style='padding: 0 10px'>%d</td>", i);
            strcat(pageBuffer, tmpBuffer);

            for (byte c = 0; c < COLORS; c++)
            {
                char * color = ALL_COLOR_CLASSES[game->guesses[i][c]];
                sprintf(tmpBuffer, "<td class='%s'>%s</td>", color, color);
                strcat(pageBuffer, tmpBuffer);
            }
            sprintf(tmpBuffer, "<td>%d</td><td>%d</td></tr>", game->guesses[i][COLORS], game->guesses[i][COLORS + 1]);
            strcat(pageBuffer, tmpBuffer);
        }

        strcat(pageBuffer, "</table>");

        // Let user make guess
        if (game->vsPlayer && game->host == user)
        {
            strcat(pageBuffer, "<p>You picked the code, you can't guess.</p>");
        }
        else
        {
            if (game->state == STATE_GAME_STARTED)
            {
                strcat(pageBuffer, "<p>Make a guess:</p><form method='get' action='play'>");
                for (byte i = 0; i < 4; i++) // Color picker 1 -> 4
                {
                    sprintf(tmpBuffer, "<select name='c%d'>", i);
                    strcat(pageBuffer, tmpBuffer);

                    for (byte c = 0; c < game->colors; c++)
                    {
                        sprintf(tmpBuffer, "<option value='%d' class='%s'>%s</option>", c, ALL_COLOR_CLASSES[c], ALL_COLOR_CLASSES[c]);
                        strcat(pageBuffer, tmpBuffer);
                    }

                    strcat(pageBuffer, "</select>");
                }
                strcat(pageBuffer, "<input type='submit' value='Choose!'/></form>");
            }
            else if (game->state == STATE_GAME_OVER)
            {
                strcat(pageBuffer, "<p>Game over! The host won!</p>");
            }
            else if (game->state == STATE_GAME_WON)
            {
                strcat(pageBuffer, "<p>Game over! The codebreakers won!</p>");
            }
            else
            {
                strcat(pageBuffer, "<p>Incorrect game state.</p>");
            }
        }
    }
    strcat(pageBuffer, pageFooter);

    CgiRequest->fHttpResponse = CgiHttpOk;
    CgiRequest->fDataType = CGIDataTypeHtml;
    CgiRequest->fResponseBufferPtr = pageBuffer;
    CgiRequest->fResponseBufferLength = strlen(pageBuffer);
}


/*************************************************************
 *                  ALL INSTALL / REMOVE LOGIC
 *************************************************************/

typedef void huge _pascal _saveregs (*CGIfn)(rpCgiPtr); // Because function pointer syntax in unreadable

char *cgiNames[] =      {/*"clock",*/           "home",          "pickUsername",            "start",            "play"};
int cgiMethods[] =      {/*CgiHttpGet,*/        CgiHttpGet,      CgiHttpPost,               CgiHttpGet,         CgiHttpGet };
CGIfn cgiFunctions[] =  {/*cgiClockFunction,*/  cgiHomeFunction, cgiPickUsernameFunction,   cgiStartFunction,   cgiPlayFunction};

void installCGIMethods()
{
    CGI_Entry cgiEntry;

    byte n = sizeof(cgiMethods) / sizeof(int);
    for (byte i = 0; i < n; i++)
    {
        cgiEntry.PathPtr = cgiNames[i];
        cgiEntry.CgiFuncPtr = cgiFunctions[i];
        cgiEntry.method = cgiMethods[i];

        if (CGI_Install(&cgiEntry) != 0)
        {
            printf("Installing CGI function %s failed\n", cgiEntry.PathPtr);
            endProgram();
        }
    }
}

void removeCGIMethods()
{
    byte n = sizeof(cgiMethods) / sizeof(int);
    for (byte i = 0; i < n; i++)
    {
        if (CGI_Delete(cgiNames[i]))
        {
            printf("Removing %s failed\n", cgiNames[i]);
        }
    }
}
