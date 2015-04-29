/*
 * mastermind.c
 *
 *  Created on: 24-apr.-2015
 *      Author: Dries007
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
    pfe_enable_bus(0xFF, 1);
    pfe_enable_pcs(1);
    pfe_enable_pio(2, 5);
    pfe_enable_pio(3, 5);
}

/**
 * Shortcut function for hal_read_bus
 */
byte readDatabus(address addr)
{
    byte bank = addr >> 8;
    pfe_enable_pio(2, bank & 0x01 ? 4 : 5);
    pfe_enable_pio(3, bank & 0x02 ? 4 : 5);
    return hal_read_bus((addr & 0x0FF) | 0x100, 0xFFFF, 0x0000);

    //selectBank(BANK_FROM_ADDRESS(addr));
    //byte value = hal_read_bus(NORMALIZE_ADDRESS(addr), 0xFFFF, 0x0000);
//#if DEBUG
    //printf("READ 0x%02x (%c) to 0x%04x @ %d (0x%04x)\n", value, value, NORMALIZE_ADDRESS(addr), BANK_FROM_ADDRESS(addr), addr);
//#endif
    //return value;
}

void writeDatabus(address addr, byte val)
{
    byte bank = addr >> 8;
    //printf("\nWriting 0x%02x to 0x%04x (Bank %d, Real address 0x%04x)\n", val, addr, bank, (addr & 0x0FF) | 0x100);
    pfe_enable_pio(2, bank & 0x01 ? 4 : 5);
    pfe_enable_pio(3, bank & 0x02 ? 4 : 5);
    hal_write_bus((addr & 0x0FF) | 0x100, val, 0xFFFF, 0x0000);

//    selectBank(BANK_FROM_ADDRESS(addr));
//    hal_write_bus(NORMALIZE_ADDRESS(addr), value, 0xFFFF, 0x0000);
//#if DEBUG
//    printf("WRITE 0x%02x (%c) to 0x%04x @ %d (0x%04x)\n", value, value, NORMALIZE_ADDRESS(addr), BANK_FROM_ADDRESS(addr), addr);
//#endif
}

/*
 * Uses cLib function:
 *
 *  pfe_enable_pio(PIO pin #, mode)
 *      Mode 4 = Output init value = High
 *      Mode 5 = Output init value = Low
 */
byte selectedBank = 0xFF;
void selectBank(byte bank)
{
    if (selectedBank != bank)
    {
//#if DEBUG
    printf("BANK %d: PIO 2 as %c, PIO 3 as %c\n", bank, bank & 0x01 ? 'H' : 'L', bank & 0x02 ? 'H' : 'L');
//#endif
        hal_write_pio(2, bank & 0x01);
        hal_write_pio(3, bank & 0x02);
        selectedBank = bank;
    }
}

/*************************************************************
 *                      LCD RELATED
 *************************************************************/

void setLCDLine(byte line, const char * format, ...)
{
    va_list aptr;
    va_start(aptr, format);

    byte buffer[LCD_LINE_SIZE + 1];
    buffer[LCD_LINE_SIZE] = 0x00;

    vsprintf(buffer, format, aptr);
    printf("Write to LCD line %d: %s\n", line, buffer);//todo: debug

    byte pos = 0x00;
    if (line == 1) pos = 0x40;
    else if (line == 2) pos = 0x14;
    else if (line == 3) pos = 0x54;
    writeDatabus(RAM_LCD_CMD, pos);
    writeDatabus(RAM_INT_SEND, CMD_LCD_POS);
    delay(1);

//    address offset = RAM_LCD_START;// + (LCD_LINE_SIZE * (line % LCD_LINES));
    address i = 0;
    for (; i < strlen(buffer); i++) // chars
    {
        writeDatabus(i + RAM_LCD_START, buffer[i]);
    }
    for (; i < LCD_LINE_SIZE; i++) // spaces
    {
        writeDatabus(i + RAM_LCD_START, ' ');
    }
    writeDatabus(LCD_LINE_SIZE + RAM_LCD_START, 0x00);

    va_end(aptr);
    writeDatabus(RAM_INT_SEND, CMD_LCD_CHAR);
    delay(10);
}

/*************************************************************
 *                      TIME RELATED
 *************************************************************/

void initTime()
{
    /*
     * HTTP GET time from http://currentmillis.com/api/seconds-since-unix-epoch.php
     */
    int temp = 0;
    char buffer[301];
    httpGet("currentmillis.com/api/seconds-since-unix-epoch.php", buffer, 300, "8.8.8.8");
    const char newline[2] = "\n";
    unsigned long int time = 0;
    char * token;
    token = strtok(buffer, newline);

    while (token != NULL)
    {
        if (temp == 0)
        {
            sscanf(token, "HTTP/1.1 %d", &temp);
            if (temp != 200)
            {
                printf("ERROR [RTC init] HTTP code != 200: %d\n", temp);
                printf("%s\n", buffer);
                return;
            }
        }
        else
        {
            sscanf(token, "%lu", &time);
            token = strtok(NULL, newline);
            if (time != 0) break;
        }
    }

    TimeDate_Structure timeDate;

    // We are well past 1999 by now
    timeDate.dcen = 20; // 20xx

    /*
     * TIME CONVERSION SOURCE: http://codereview.stackexchange.com/questions/38275/convert-between-date-time-and-time-stamp-without-using-std-library-routines
     * HAS BEEN ADJUSTED: added +1day for 2000 being a leap year and -30yrs for linux epoch
     * ONLY ACCURATE FROM 2000 to 2099
     */

    // Easy part
    timeDate.sec = time % 60;
    time /= 60;
    timeDate.min = time % 60;
    time /= 60;
    timeDate.hr = TIMEZONE_OFFSET + (time % 24);
    time /= 24;

    // Now the years: Uses amount of days per 4 years
    byte years = (time / (365 * 4 + 1) * 4) - 30;
    time %= (365 * 4 + 1); // Time is now in days!
    // 1 day for 2000 as leap year
    time += 1;

    // Magic numbers aka (days per month, per 4 years)
    const static unsigned short days[4][12] = {
            { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335 },
            { 366, 397, 425, 456, 486, 517, 547, 578, 609, 639, 670, 700 },
            { 731, 762, 790, 821, 851, 882, 912, 943, 974, 1004, 1035, 1065 },
            { 1096, 1127, 1155, 1186, 1216, 1247, 1277, 1308, 1339, 1369, 1400, 1430 }
    };

    byte year;
    for (year = 3; year > 0; year--)
    {
        if (time >= days[year][0]) break;
    }

    byte month;
    for (month = 11; month > 0; month--)
    {
        if (time >= days[year][month]) break;
    }

    timeDate.yr = years + year;
    timeDate.mn = month + 1;
    timeDate.dy = time - days[year][month] + 1;

    printf("System clock set to: %02d%02d-%02d-%02d @ %02d:%02d:%02d\n", timeDate.dcen, timeDate.yr, timeDate.mn, timeDate.dy, timeDate.hr, timeDate.min, timeDate.sec);
    RTX_Set_TimeDate(&timeDate); // Save to system clock
}

/****************************************************************************************************
 *                                              TASKS
 ****************************************************************************************************/

byte LCDupdateRunning;

void LCDupdate()
{
    LCDupdateRunning = 1;
    byte ip[16];
    TimeDate_Structure timeDate;

    while (LCDupdateRunning)
    {
        Get_IPConfig(ip, NULL, NULL);
        if (LCD_LINE_SIZE >= 20) setLCDLine(2, "IP: %s", ip);
        else setLCDLine(2, "%s", ip);

        RTX_Get_TimeDate(&timeDate);
        if (LCD_LINE_SIZE >= 19) setLCDLine(3, "%02d%02d-%02d-%02d %02d:%02d:%02d", timeDate.dcen, timeDate.yr, timeDate.mn, timeDate.dy, timeDate.hr, timeDate.min, timeDate.sec);
        else setLCDLine(3, "%02d-%02d-%02d %02d:%02d", timeDate.yr, timeDate.mn, timeDate.dy, timeDate.hr, timeDate.min);

        RTX_Sleep_Time(900);
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

    setLCDLine(0, "TEST Line 0"); delay(1000);
    setLCDLine(1, "TEST Line 1"); delay(1000);
    setLCDLine(2, "TEST Line 2"); delay(1000);
    setLCDLine(3, "TEST Line 3"); delay(1000);
}

void main()
{
    // Get focus
    BIOS_Set_Focus(FOCUS_APPLICATION);

    /**
     * INIT All of the things!
     */
    enableDatabus();

    printf("\n\nAVR firmware version id: %d 0x%02x\n\n\n", readDatabus(RAM_VERSION_1), readDatabus(RAM_VERSION_2));

    // Setup AVR for LCD
    //initLcd();

    // Print firmware info to LCD
    setLCDLine(0, "(C) Dries007 '%s", CYEAR);
    //setLCDLine(1, "Version %s.%d", VERSION, readDatabus(ADDRESS_FW_VER));

    // Ethernet connection check
    if (BIOS_Ethernet_State(NULL, NULL))
    {
        setLCDLine(0, "ERROR");
        setLCDLine(1, "NO ETHERNET!");
        return;
    }

    // Do time setup
    printf("Fetching time...\n");
    initTime();

    /**
     * RUN ALL TASKS
     *
    int result = RTX_Create_Task(&LCDupdateID , &LCDupdateTaskDefBlock);

    if (result!=0)
    {
        printf("Creating/restart LCDupdate failed %d, exit program\n", result);
        //delete task1
        RTX_Delete_Task(LCDupdateID);
        endProgram();
    }

    /**
     * CGI methods
     */

    printf("Installing CGI methods\n");
    installCGIMethods();

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
        printf("[T] Redo time update\n");
        printf("[U] Print all known users\n");
        printf("[I] Interrupt test to AVR\n");
        printf("[L] Print Time and IP to LCd\n");

        scanf("%c%*c", &key);
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
        case 'T':
            initTime();
            break;
        case 'U':
            printAllUsers();
            break;
        case 'I':
            test();
            break;
        case 'L':
        {
            byte ip[16];
            TimeDate_Structure timeDate;

            Get_IPConfig(ip, NULL, NULL);
            if (LCD_LINE_SIZE >= 20) setLCDLine(2, "IP: %s", ip);
            else setLCDLine(2, "%s", ip);

            RTX_Get_TimeDate(&timeDate);
            if (LCD_LINE_SIZE >= 19) setLCDLine(3, "%02d%02d-%02d-%02d %02d:%02d:%02d", timeDate.dcen, timeDate.yr, timeDate.mn, timeDate.dy, timeDate.hr, timeDate.min, timeDate.sec);
            else setLCDLine(3, "%02d-%02d-%02d %02d:%02d", timeDate.yr, timeDate.mn, timeDate.dy, timeDate.hr, timeDate.min);
        }
            break;
        /*
        {
            char buffer[80];
            scanf("%s%*c", &buffer);
            for (address i = 0; i < 80; i++)
            {
                writeDatabus(i + RAM_LCD_START, buffer[i]);
            }
            writeDatabus(RAM_LCD_END, 0x00);
            writeDatabus(RAM_INT_SEND, CMD_LCD_CL_PR);
        }
        break;
        */
        default:
            printf("Char not in menu: %c\n", key);
        }
    }
}

/****************************************************************************************************
 *                                              WEB
 ****************************************************************************************************/

/*************************************************************
 *                      CLOCK (debug)
 *************************************************************/

void huge _pascal _saveregs cgiClockFunction(rpCgiPtr CgiRequest)
{
    static char pageBuffer[2048];  // Buffer to contain web page
    char tmpBuffer[512]; // Buffer for string manipulation functions

    TimeDate_Structure timeDate;
    RTX_Get_TimeDate(&timeDate);

    sprintf(pageBuffer, "<html><head><meta http-equiv=\"refresh\" content=\"1\"><title>");

    if (timeDate.sec % 2) strcat(pageBuffer, "Tick</title></head><body><h1>");
    else strcat(pageBuffer, "Tock</title></head><body><h1>");

    sprintf(tmpBuffer, "%02d%02d-%02d-%02d @ %02d:%02d:%02d\n", timeDate.dcen, timeDate.yr, timeDate.mn, timeDate.dy, timeDate.hr, timeDate.min, timeDate.sec);

    strcat(pageBuffer, tmpBuffer);
    strcat(pageBuffer, "</h1></body></html>");

    CgiRequest->fHttpResponse = CgiHttpOk;
    CgiRequest->fDataType = CGIDataTypeHtml;
    CgiRequest->fResponseBufferPtr = pageBuffer;
    CgiRequest->fResponseBufferLength = strlen(pageBuffer);
}

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

char *cgiNames[] =      {"clock",           "home",          "pickUsername",            "start",            "play"};
int cgiMethods[] =      {CgiHttpGet,        CgiHttpGet,      CgiHttpPost,               CgiHttpGet,         CgiHttpGet };
CGIfn cgiFunctions[] =  {cgiClockFunction,  cgiHomeFunction, cgiPickUsernameFunction,   cgiStartFunction,   cgiPlayFunction};

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
