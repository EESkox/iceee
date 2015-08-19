#pragma warning(disable: 4996)

#include "Debug.h"

#include "Util.h"
#include "StringList.h"
#include "Globals.h"
#include "Config.h"
#include "DebugTracer.h"
#include "ByteBuffer.h"
#include <math.h>
#include <stdlib.h>

ChangeData SessionVarsChangeData;

// For debugging when needed

//char XLogBuffer[4096];  //Holds a constructed log message generated by this thread
//
//void LogMessageL(unsigned short messageType, const char *format, ...)
//{
//	if(g_Log.LoggingEnabled == false)
//		return;
//
//	if(g_SimulatorLog == false)
//		return;
//
//	if(!(g_Log.Filter & messageType))
//		return;
//
//	sprintf(XLogBuffer, "[Sim:%d] ", 1);
//	int pos = strlen(XLogBuffer);
//
//	va_list args;
//	va_start (args, format);
//	Util::SafeFormatArg(&XLogBuffer[pos], sizeof(XLogBuffer) - pos, format, args);
//	va_end (args);
//
//	g_Log.AddMessage(XLogBuffer);
//}



int PrepExt_Broadcast(char *buffer, const char *message)
{
	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 0);       //_handleInfoMsg
	wpos += PutShort(&buffer[wpos], 0);      //Placeholder for size

	wpos += PutStringUTF(&buffer[wpos], message);
	wpos += PutByte(&buffer[wpos], 14);       //Event type for broadcast

	PutShort(&buffer[1], wpos - 3);     //Set size
	return wpos;
}



int PrepExt_SetAvatar(char *buffer, int creatureID)
{
	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 0x04);
	wpos += PutShort(&buffer[wpos], 0x00);

	wpos += PutInteger(&buffer[wpos], creatureID);
	wpos += PutByte(&buffer[wpos], 0x01);    //Event to Set Avatar

	PutShort(&buffer[1], wpos - 3);       //Set message size
	return wpos;
}

int PrepExt_SetTimeOfDay(char *buffer, char *envType)
{
	int wpos = 0;

	wpos += PutByte(&buffer[wpos], 42);   //_handleEnvironmentUpdateMsg
	wpos += PutShort(&buffer[wpos], 0);

	wpos += PutByte(&buffer[wpos], 2);   //Mask for time of day update

	wpos += PutStringUTF(&buffer[wpos], "");    //zoneID
	wpos += PutInteger(&buffer[wpos], 0);   //zoneDefID
	wpos += PutShort(&buffer[wpos], 0);  //zonePageSize
	wpos += PutStringUTF(&buffer[wpos], "");   //Terrain
	wpos += PutStringUTF(&buffer[wpos], envType);   //envtype
	//When the mask is 2, the function changes the time of day using the EnvironmentType string
	//the function returns from that point and never gets to the rest of the map stuff.

	PutShort(&buffer[1], wpos - 3);       //Set message size
	return wpos;
}



int PrepExt_AbilityEvent(char *buffer, int creatureID, int abilityID, int abilityEvent)
{
	//Same as AbilityActivate, but target lists and ground are always zero.
	//Used for the utility messages such as activation requests.

	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 60);  //_handleAbilityActivationMsg
	wpos += PutShort(&buffer[wpos], 0);

	wpos += PutInteger(&buffer[wpos], creatureID);   //actorID
	wpos += PutShort(&buffer[wpos], abilityID);     //abId
	wpos += PutByte(&buffer[wpos], abilityEvent);      //event

	wpos += PutInteger(&buffer[wpos], 0);   //target_len
	wpos += PutInteger(&buffer[wpos], 0);   //secondary_len
	wpos += PutByte(&buffer[wpos], 0);      //has_ground

	PutShort(&buffer[1], wpos - 3);
	return wpos;
}

int PrepExt_SendAbilityOwn(char *buffer, int CID, int abilityID, int eventID)
{
	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 60);  //_handleAbilityActivationMsg
	wpos += PutShort(&buffer[wpos], 0);

	wpos += PutInteger(&buffer[wpos], CID);   //Creature Instance ID
	wpos += PutShort(&buffer[wpos], abilityID);  //ability ID
	wpos += PutByte(&buffer[wpos], eventID);  //7 = ability ownage
	wpos += PutInteger(&buffer[wpos], 0);   //target_len
	wpos += PutInteger(&buffer[wpos], 0);   //secondary_len
	wpos += PutByte(&buffer[wpos], 0);      //has_ground

	PutShort(&buffer[1], wpos - 3);       //Set message size
	return wpos;
}

int PrepExt_CancelUseEvent(char *buffer, int CreatureID)
{
	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 4);  //_handleCreatureEventMsg
	wpos += PutShort(&buffer[wpos], 0);  //size
	wpos += PutInteger(&buffer[wpos], CreatureID);
	wpos += PutByte(&buffer[wpos], 11);  //creature "used" event
	wpos += PutStringUTF(&buffer[wpos], "");
	wpos += PutFloat(&buffer[wpos], -1.0F);  //A delay of -1 will interrupt the action
	PutShort(&buffer[1], wpos - 3);  //size
	return wpos;
}

int PrepExt_ActorJump(char *buffer, int actor)
{
	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 4);  //_handleCreatureEventMsg
	wpos += PutShort(&buffer[wpos], 0);

	wpos += PutInteger(&buffer[wpos], actor);   //actorID
	wpos += PutByte(&buffer[wpos], 3);     //3 = jump

	PutShort(&buffer[1], wpos - 3);       //Set message size
	return wpos;
}

int PrepExt_RemoveCreature(char *buffer, int actorID)
{
	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 4);  //_handleCreatureEventMsg
	wpos += PutShort(&buffer[wpos], 0);

	wpos += PutInteger(&buffer[wpos], actorID);   //actorID
	wpos += PutByte(&buffer[wpos], 0);     //0 = remove ID

	PutShort(&buffer[1], wpos - 3);       //Set message size
	return wpos;
}

int PrepExt_SendInfoMessage(char *buffer, const char *message, unsigned char eventID)
{
	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 0);       //_handleInfoMsg
	wpos += PutShort(&buffer[wpos], 0);      //Placeholder for size

	wpos += PutStringUTF(&buffer[wpos], message);
	wpos += PutByte(&buffer[wpos], eventID);       //Event type for error message

	PutShort(&buffer[1], wpos - 3);     //Set size
	return wpos;
}

int PrepExt_SendFallDamage(char *buffer, int damage)
{
	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 0);       //_handleInfoMsg
	wpos += PutShort(&buffer[wpos], 0);      //Placeholder for size

	wpos += PutStringUTF(&buffer[wpos], "");  //Unused?
	wpos += PutByte(&buffer[wpos], 11);       //Event type for fall damage message
	wpos += PutInteger(&buffer[wpos], damage);

	PutShort(&buffer[1], wpos - 3);     //Set size
	return wpos;
}

int PrepExt_GenericChatMessage(char *buffer, int creatureID, const char *name, const char *channel, const char *message)
{
	//Handles a "communicate" (0x04) message containing a channel and string
	//Sends back a "_handleCommunicationMsg" (50 = 0x32) to the client
	//Ideally this function should broadcast the message to all active player threads
	//in the future so that all players can receive the communication message.

	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 0x32);       //_handleCommunicationMsg
	wpos += PutShort(&buffer[wpos], 0);         //Placeholder for size

	wpos += PutInteger(&buffer[wpos], creatureID);    //Character ID who's sending the message
	wpos += PutStringUTF(&buffer[wpos], name);     //Character name
	wpos += PutStringUTF(&buffer[wpos], channel);  //channel type
	wpos += PutStringUTF(&buffer[wpos], message);  //message body

	PutShort(&buffer[1], wpos - 3);     //Set size
	return wpos;
}


int PrepExt_CooldownExpired(char *buffer, long actor, const char *cooldownCategory)
{
	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 4);  //_handleCreatureEventMsg
	wpos += PutShort(&buffer[wpos], 0);

	wpos += PutInteger(&buffer[wpos], actor);   //actorID
	wpos += PutByte(&buffer[wpos], 22);     //22 = Cooldown category expired

	wpos += PutStringUTF(&buffer[wpos], cooldownCategory);

	PutShort(&buffer[1], wpos - 3);       //Set message size
	return wpos;
}

int PrepExt_ChangeTarget(char *buffer, int sourceID, int targetID)
{
	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 4);  //_handleCreatureEventMsg
	wpos += PutShort(&buffer[wpos], 0);

	wpos += PutInteger(&buffer[wpos], sourceID);
	wpos += PutByte(&buffer[wpos], 5);     //5 = Entity's target changed
	wpos += PutInteger(&buffer[wpos], targetID);

	PutShort(&buffer[1], wpos - 3);       //Set message size
	return wpos;
}

int PrepExt_ExperienceGain(char *buffer, int CreatureID, int ExpAmount)
{
	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 4);  //_handleCreatureEventMsg
	wpos += PutShort(&buffer[wpos], 0);

	wpos += PutInteger(&buffer[wpos], CreatureID);
	wpos += PutByte(&buffer[wpos], 21);     //21 = Experience Gained

	wpos += PutInteger(&buffer[wpos], ExpAmount);

	if(g_ProtocolVersion >= 38)
	{
		//Another byte determines the source of the experience.
		// 0 = none    "You gained +x exp."
		// 1 = kill    "...from slaying a foe"
		// 2 = quest   "...for completing a quest"
		wpos += PutByte(&buffer[wpos], 0);
	}

	PutShort(&buffer[1], wpos - 3);       //Set message size
	return wpos;
}

int PrepExt_QueryResponseNull(char *buffer, int queryIndex)
{
	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 1);              //_handleQueryResultMsg
	wpos += PutShort(&buffer[wpos], 0);             //Placeholder for message size
	wpos += PutInteger(&buffer[wpos], queryIndex);  //Query response index
	wpos += PutShort(&buffer[wpos], 0);             //Row count
	PutShort(&buffer[1], wpos - 3);                 //Message size
	return wpos;
}

int PrepExt_QueryResponseString(char *buffer, int queryIndex, const char *strData)
{
	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 1);              //_handleQueryResultMsg
	wpos += PutShort(&buffer[wpos], 0);             //Placeholder for message size
	wpos += PutInteger(&buffer[wpos], queryIndex);  //Query response index
	wpos += PutShort(&buffer[wpos], 1);             //Row count
	wpos += PutByte(&buffer[wpos], 1);              //String count
	wpos += PutStringUTF(&buffer[wpos], strData);   //String data
	PutShort(&buffer[1], wpos - 3);                 //Message size
	return wpos;
}

int PrepExt_QueryResponseString2(char *buffer, int queryIndex, const char *strData1, const char *strData2)
{
	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 1);              //_handleQueryResultMsg
	wpos += PutShort(&buffer[wpos], 0);             //Placeholder for message size
	wpos += PutInteger(&buffer[wpos], queryIndex);  //Query response index
	wpos += PutShort(&buffer[wpos], 1);             //Row count
	wpos += PutByte(&buffer[wpos], 2);              //String count
	wpos += PutStringUTF(&buffer[wpos], strData1);   //String data
	wpos += PutStringUTF(&buffer[wpos], strData2);   //String data
	PutShort(&buffer[1], wpos - 3);                 //Message size
	return wpos;
}

int PrepExt_QueryResponseStringList(char *buffer, int queryIndex, const STRINGLIST &strData)
{
	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 1);              //_handleQueryResultMsg
	wpos += PutShort(&buffer[wpos], 0);             //Placeholder for message size
	wpos += PutInteger(&buffer[wpos], queryIndex);  //Query response index
	wpos += PutShort(&buffer[wpos], 1);             //Row count

	int count = strData.size();
	if(count > 255)
	{
		g_Log.AddMessageFormat("[WARNING] PrepExt_QueryResponseStringList too many strings: %d", count);
		count = 255;
	}
	wpos += PutByte(&buffer[wpos], count); //String count
	for(int a = 0; a < count; a++)
		wpos += PutStringUTF(&buffer[wpos], strData[a].c_str());   //String data
	PutShort(&buffer[1], wpos - 3);                 //Message size
	return wpos;
}

int PrepExt_QueryResponseMultiString(char *buffer, int queryIndex, const MULTISTRING &strData)
{
	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 1);              //_handleQueryResultMsg
	wpos += PutShort(&buffer[wpos], 0);             //Placeholder for message size
	wpos += PutInteger(&buffer[wpos], queryIndex);  //Query response index

	size_t rowCount = strData.size();
	size_t stringCount;
	wpos += PutShort(&buffer[wpos], rowCount);
	for(size_t rows = 0; rows < rowCount; rows++)
	{
		stringCount = strData[rows].size();
		if(stringCount > 255)
		{
			g_Log.AddMessageFormat("[WARNING] PrepExt_QueryResponseMultiString too many strings: %d", stringCount);
			stringCount = 255;
		}
		wpos += PutByte(&buffer[wpos], stringCount);
		for(size_t str = 0; str < stringCount; str++)
		{
			//g_Log.AddMessageFormat("[DEBUG] [%d][%d]=%s", rows, str, strData[rows][str].c_str());
			wpos += PutStringUTF(&buffer[wpos], strData[rows][str].c_str());   //String data
		}
	}
	PutShort(&buffer[1], wpos - 3);
	return wpos;
}

int PrepExt_QueryResponseError(char *buffer, int queryIndex, const char *message)
{
	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 1);              //_handleQueryResultMsg
	wpos += PutShort(&buffer[wpos], 0);             //Placeholder for message size
	wpos += PutInteger(&buffer[wpos], queryIndex);  //Query response index
	wpos += PutShort(&buffer[wpos], 0x7000);        //Negative number indicates error
	wpos += PutStringUTF(&buffer[wpos], message);   //String data
	PutShort(&buffer[1], wpos - 3);                 //Message size
	return wpos;
}

int PrepExt_SendEffect(char *buffer, int sourceID, const char *effectName, int targetID)
{
	//Send an effect to the client.  Send as a single target effect
	//unless targetID is nonzero.

	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 4);       //_handleCreatureEventMsg
	wpos += PutShort(&buffer[wpos], 0);      //Reserve for size

	wpos += PutInteger(&buffer[wpos], sourceID);

	wpos += PutByte(&buffer[wpos], (targetID == 0) ? 4 : 12);  //Cue effect
	wpos += PutStringUTF(&buffer[wpos], effectName);
	if(targetID != 0)
		wpos += PutInteger(&buffer[wpos], targetID);

	PutShort(&buffer[1], wpos - 3);       //Set message size
	return wpos;
}

int PrepExt_SendHeartbeatMessage(char *buffer, unsigned int elapsedMilliseconds)
{
	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 90);   //_handleHeartbeatMessage
	wpos += PutShort(&buffer[wpos], 0);   //Reserve for size
	wpos += PutInteger(&buffer[wpos], elapsedMilliseconds);
	PutShort(&buffer[1], wpos - 3);       //Set message size
	return wpos;
}

int PrepExt_SendAdvancedEmote(char *buffer, int creatureID, const char *emoteName, float emoteSpeed, int loop)
{
	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 100);   //_handleModMessage   REQUIRES MODDED CLIENT
	wpos += PutShort(&buffer[wpos], 0);    //Reserve for size

	wpos += PutByte(&buffer[wpos], MODMESSAGE_EVENT_EMOTE);   //event for advanced emote

	wpos += PutInteger(&buffer[wpos], creatureID);
	wpos += PutStringUTF(&buffer[wpos], emoteName);
	wpos += PutFloat(&buffer[wpos], emoteSpeed);
	wpos += PutByte(&buffer[wpos], (loop != 0));

	PutShort(&buffer[1], wpos - 3);       //Set message size
	return wpos;
}

int PrepExt_SendEmoteControl(char *buffer, int creatureID, int emoteEvent)
{
	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 100);   //_handleModMessage   REQUIRES MODDED CLIENT
	wpos += PutShort(&buffer[wpos], 0);    //Reserve for size

	wpos += PutByte(&buffer[wpos], MODMESSAGE_EVENT_EMOTE_CONTROL);   //event for advanced emote

	wpos += PutInteger(&buffer[wpos], creatureID);
	wpos += PutByte(&buffer[wpos], emoteEvent);

	PutShort(&buffer[1], wpos - 3);       //Set message size
	return wpos;
}

int PrepExt_ModStopSwimFlag(char *buffer)
{
	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 100);   //_handleModMessage   REQUIRES MODDED CLIENT
	wpos += PutShort(&buffer[wpos], 0);    //Reserve for size

	wpos += PutByte(&buffer[wpos], MODMESSAGE_EVENT_STOP_SWIM);  //Event code

	PutShort(&buffer[1], wpos - 3);       //Set message size
	return wpos;
}




int PrepExt_TradeCurrencyOffer(char *buffer, int offeringPlayerID, int tradeAmount)
{
	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 51);     //_handleTradeMsg
	wpos += PutShort(&buffer[wpos], 0);     //Placeholder for size
	wpos += PutInteger(&buffer[wpos], offeringPlayerID);   //traderID
	wpos += PutByte(&buffer[wpos], TradeEventTypes::CURRENCY_OFFERED);     //eventType
	wpos += PutByte(&buffer[wpos], 1);   //currency types
	wpos += PutByte(&buffer[wpos], CurrencyCategory::COPPER);     //currencies offered
	wpos += PutInteger(&buffer[wpos], tradeAmount);                    //currency amount
	PutShort(&buffer[1], wpos - 3);       //Set message size
	return wpos;
}

int PrepExt_CreatureEventPortalRequest(char *buffer, int actorID, const char *casterName, const char *locationName)
{
	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 4);  //_handleCreatureEventMsg
	wpos += PutShort(&buffer[wpos], 0);

	wpos += PutInteger(&buffer[wpos], actorID);
	wpos += PutByte(&buffer[wpos], 24);             //event to request a portal

	wpos += PutStringUTF(&buffer[wpos], casterName);
	wpos += PutStringUTF(&buffer[wpos], locationName);

	PutShort(&buffer[1], wpos - 3);       //Set message size
	return wpos;
}

int PrepExt_CreatureEventVaultSize(char *buffer, int actorID, int vaultSize, int deliverySlots)
{
	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 4);  //_handleCreatureEventMsg
	wpos += PutShort(&buffer[wpos], 0);

	wpos += PutInteger(&buffer[wpos], actorID);
	wpos += PutByte(&buffer[wpos], 25);             //event to update vault size

	wpos += PutInteger(&buffer[wpos], vaultSize);
	wpos += PutInteger(&buffer[wpos], deliverySlots);

	PutShort(&buffer[1], wpos - 3);       //Set message size
	return wpos;
}

int PrepExt_SendTimeOfDayMsg(char *buffer, const char *envType)
{
	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 42);   //_handleEnvironmentUpdateMsg
	wpos += PutShort(&buffer[wpos], 0);

	wpos += PutByte(&buffer[wpos], 2);   //Mask: Time of day update event

	wpos += PutStringUTF(&buffer[wpos], "");    //zoneIDString
	wpos += PutInteger(&buffer[wpos], 0);      //zoneDefID
	wpos += PutShort(&buffer[wpos], 0);  //zonePageSize
	wpos += PutStringUTF(&buffer[wpos], "");   //Terrain
	wpos += PutStringUTF(&buffer[wpos], envType);   //envtype
	wpos += PutStringUTF(&buffer[wpos], "");   //mapName

	PutShort(&buffer[1], wpos - 3);       //Set message size
	return wpos;
}


int randmod(int max) {
	// Max is exclusive, e.g, max of 10 would give numbers between 0 and 9
	return rand()%max;
}

int randmodrng(int min, int max) {
	// Min is inclusive, max is exclusive, e.g, min of 3, max of 10 would give numbers between 3 and 9
	return(rand()%(max-min)+min);
}

int randi(int max) {
	return randint(1, max);
}

int randint(int min, int max)
{
	//Returning <max> is possible, but highly unlikely compared to the individual
	//chances of any other value
	//return ((double) rand() / ((double) RAND_MAX) * (max - min)) + min;

	//This should be fixed to generate <max> as much as others
	return (int) (((double) rand() / ((double)RAND_MAX + 1) * ((max + 1) - min)) + min);
}

double randdbl(double min, double max)
{
	return ((double)rand() / ((double)RAND_MAX) * (max - min)) + min;
}

char *StringFromInt(char *buffer, int value)
{
	sprintf(buffer, "%d", value);
	return buffer;
}

char *StringFromFloat(char *buffer, float value)
{
	sprintf(buffer, "%g", value);
	return buffer;
}

char *StringFromBool(char *buffer, bool value)
{
	sprintf(buffer, "%s", (value == true) ? "true" : "false");
	return buffer;
}

char *StringFromBool(char *buffer, int value)
{
	//Some integers are used as boolean values.  Avoids compiler type warnings.
	sprintf(buffer, "%s", (value != 0) ? "true" : "false");
	return buffer;
}

/*
StringWrite :: StringWrite(char *buffer, int size)
{
	writeBuf = buffer;
	bufSize = size;
	maxSafeLen = bufSize - 1;
	writePos = 0;
	overflow = false;
}

StringWrite :: ~StringWrite()
{
}

int StringWrite :: PutChar(char value)
{
	//Need an extra space to set the null terminator.
	if(writePos >= bufSize - 1)
	{
		g_Log.AddMessageFormatW(MSG_SHOW, "[STRING] PutChar() buffer is full");
		overflow = true;
		return 0;
	}
	writeBuf[writePos++] = value;
	writeBuf[writePos] = 0;
	return 1;
}

int StringWrite :: Format(const char *format, ...)
{
	writePos = 0;
	va_list args;
	va_start (args, format);
	int write = vsnprintf(writeBuf, maxSafeLen, format, args);
	va_end (args);

	if(write < 0)
		write = 0;
	writePos += write;
	if(writePos > maxSafeLen)
	{
		overflow = true;
		g_Log.AddMessageFormatW(MSG_SHOW, "[STRING] Format() buffer is full");
	}
	return write;
}

int StringWrite :: FormatPos(const char *format, ...)
{
	if(writePos >= maxSafeLen)
	{
		overflow = true;
		return 0;
	}

	va_list args;
	va_start (args, format);
	int write = vsnprintf(&writeBuf[writePos], maxSafeLen - writePos, format, args);
	va_end (args);

	if(write < 0)
		write = 0;
	writePos += write;
	if(writePos > maxSafeLen)
	{
		overflow = true;
		g_Log.AddMessageFormatW(MSG_SHOW, "[STRING] FormatPos() buffer is full");
	}
	return write;
}

bool StringWrite :: Overflow(void)
{
	return (writePos >= (maxSafeLen - OVERFLOW_WARN));
}
*/


ChangeData :: ChangeData()
{
	FirstChange = 0;
	PendingChanges = 0;
	LastChange = 0;
}

void ChangeData :: AddChange(void)
{
	PendingChanges++;
	LastChange = g_ServerTime;
	if(PendingChanges == 1)
		FirstChange = LastChange;
}

bool ChangeData :: IsLastChangeSince(unsigned long milliseconds)
{
	if(g_ServerTime - LastChange >= milliseconds)
		return true;

	return false;
}

bool ChangeData :: CheckUpdateAndClear(unsigned long milliseconds)
{
	//If an update is pending, and a certain duration has passed,
	//return true.  The calling function should perform whatever
	//actions are necessary since the pending data will be cleared.
	if(PendingChanges == 0)
		return false;

	if(g_ServerTime - FirstChange >= milliseconds)
	{
		ClearPending();
		return true;
	}
	return false;
}

void ChangeData :: ClearPending(void)
{
	PendingChanges = 0;
	LastChange = g_ServerTime;
	FirstChange = 0;
}

namespace Util
{

void WriteString(FILE *output, const char *label, std::string &str)
{
	if(str.size() == 0)
		return;
	fprintf(output, "%s=%s\r\n", label, str.c_str());
}

void WriteString(FILE *output, const char *label, const char *str)
{
	if(strlen(str) == 0)
		return;
	fprintf(output, "%s=%s\r\n", label, str);
}

void WriteInteger(FILE *output, const char *label, int value)
{
	if(value == 0)
		return;
	fprintf(output, "%s=%d\r\n", label, value);
}

void WriteIntegerIfNot(FILE *output, const char *label, int value, int ignoreVal)
{
	if(value == ignoreVal)
		return;
	fprintf(output, "%s=%d\r\n", label, value);
}

void WriteAutoSaveHeader(FILE *output)
{
	time_t curTime = time(NULL);
	tm *timeData = localtime(&curTime);
	char timeBuf[256];
	strftime(timeBuf, sizeof(timeBuf), "%b %d %Y, %I:%M %p", timeData);
	fprintf(output, "; This file was auto saved on %s\r\n\r\n", timeBuf);
}

FILE * OpenSaveFile(const char *filename)
{
	FILE *file = fopen(filename, "wb");
	if(file == NULL)
		g_Log.AddMessageFormatW(MSG_ERROR, "[ERROR] Could not open file [%s] for writing.", filename);
	return file;
}

void Join(std::vector<std::string> &src, const char *delim, std::string &dest)
{
	std::string s = delim;
	dest.clear();
	for(uint i = 0 ; i < src.size(); i++) {
		if(i > 0)
			dest.append(s);
		dest.append(src[i]);
	}
}

int Split(const std::string &source, const char *delim, std::vector<std::string> &dest)
{
	if(dest.size() > 0)
		dest.clear();
	size_t pos = 0;
	size_t fpos = 0;
	string extract;
	do
	{
		fpos = source.find(delim, pos);
		if(fpos != string::npos)
		{
			extract = source.substr(pos, fpos - pos);
			dest.push_back(extract);
			pos = fpos + 1;
		}
	} while(fpos != string::npos);
	extract = source.substr(pos, string::npos);
	dest.push_back(extract);

	return dest.size();
}

void Replace(std::string &source, char find, char replace)
{
	size_t pos = 0;
	do
	{
		pos = source.find(find, pos);
		if(pos != string::npos)
			source[pos] = replace;
	} while(pos != string::npos);
}

typedef std::vector<std::string> STRINGLIST;
typedef std::vector<STRINGLIST> MULTISTRING;

void StringAppendInt(std::string &dest, int value)
{
	char buffer[32];
	sprintf(buffer, "%d", value);
	dest.append(buffer);
}

void StringAppendFloat(std::string &dest, float value)
{
	char buffer[32];
	sprintf(buffer, "%g", value);
	dest.append(buffer);
}

void StringAppendQuote(std::string &dest, const char *appendStr)
{
	dest.append("\"");
	dest.append(appendStr);
	dest.append("\"");
}

void SafeCopy(char *dest, const char *source, int destSize)
{
	// Copy a null-terminated string.  The copy will honor the
	// destination buffer size, truncating the result if
	// necessary, and always leaves room for a null terminator.

	if(dest == NULL)
		return;
	if(destSize == 0)
		return;
	if(source == NULL)
	{
		if(destSize > 0)
			dest[0] = 0;
		return;
	}

	int len = strlen(source);
	int max = destSize - 1;
	if(len >= max)
	{
#ifdef _DEBUG
		g_Log.AddMessageFormat("[STRING] WARNING: SafeCopy() at maximum limit to hold [%s]", source);
#endif
		strncpy(dest, source, max);
		dest[max] = 0;
	}
	else
	{
		strcpy(dest, source);
	}
}

void SafeCopyN(char *dest, const char *source, int destSize, int copySize)
{
	// Copy a portion of a string.  The copy will honor the
	// destination buffer size, truncating the result if
	// necessary, and always leaves room for a null terminator.

	if(dest == NULL)
		return;
	if(destSize == 0)
		return;
	if(source == NULL)
	{
		if(destSize > 0)
			dest[0] = 0;
		return;
	}
	if(copySize < 1)
	{
		g_Log.AddMessageFormat("[STRING] WARNING: SafeCopyN() cannot copy %d bytes", copySize);
		dest[0] = 0;
		return;
	}

	int len = copySize;
	int max = destSize - 1;
	if(len > (int)strlen(source))
		len = strlen(source);
	if(len > max)
	{
#ifdef _DEBUG
		g_Log.AddMessageFormat("[STRING] WARNING: SafeCopyN() at maximum limit to hold [%s] with [%d] bytes", source, copySize);
#endif
		len = max;
	}
	strncpy(dest, source, len);
	dest[len] = 0;
}


int IsStringTerminated(const char *buffer, int bufferSize)
{
	//Check to see if the last character in a string buffer is null terminated.
	return (buffer[bufferSize - 1] == 0);
}

int SafeFormat(char *destBuf, size_t maxCount, const char *format, ...)
{
	//Safely write formatted output to a buffer.  Return the number of characters
	//written.
	
	//Note: Guarantee room for the null terminator.
	int maxSafeSize = maxCount - 1;
	va_list args;
	va_start (args, format);
	int res = vsnprintf(destBuf, maxSafeSize, format, args);
	va_end (args);

	//Guarantee that the result is null terminated.  Microsoft Visual C++ won't
	//null-terminate if the characters written has reached the provided limit.
	destBuf[maxSafeSize] = 0;

	//MSVC returns -1 if truncated.
	//GCC returns the number of bytes required to store the result (not including NULL terminator)
	bool trunc = false;
	if(res < 0)
	{
		res = strlen(destBuf);
		trunc = true;
	}
	else if(res > maxSafeSize)
	{
		res = maxSafeSize;
		trunc = true;
	}
	if(trunc == true)
		g_Log.AddMessageFormat("[STRING] SafeFormat() output was truncated [%s]", destBuf);

	return res;
}

int SafeFormatArg(char *destBuf, size_t maxCount, const char *format, va_list argList)
{
	//Wrapper for the vsnprintf() function to help resolve any inconsistencies
	//between platforms.  For example, MSVC returns -1 on truncation and won't
	//place a null terminator if it would be truncated, while GCC returns the
	//number of characters that would normally have been written, and null
	//terminates automatically.

	//Also log any formatting attempts that have been truncated so that bugs might
	//be tracked easier.
	int maxSafeSize = maxCount - 1;
	int res = vsnprintf(destBuf, maxSafeSize, format, argList);
	destBuf[maxSafeSize] = 0;
	bool trunc = false;
	if(res < 0)
	{
		res = strlen(destBuf);
		trunc = true;
	}
	else if(res > maxSafeSize)
	{
		res = maxSafeSize;
		trunc = true;
	}
	if(trunc == true)
		g_Log.AddMessageFormat("[STRING] SafeFormatArg() output was truncated [%s]", destBuf);

	return res;
}


// Safely format printed output to a positional offset within a buffer.
int SafeFormatOffset(unsigned int offset, char *buffer, unsigned int bufferSize, const char *format, ...)
{
	int maxSafeSize = bufferSize - 1;
	if((int)offset >= maxSafeSize)
	{
#ifdef _DEBUG
		g_Log.AddMessageFormatW(MSG_WARN, "[STRING] SafeFormatOffset() string already full");
#endif
		return 0;
	}
	va_list args;
	va_start (args, format);
	int write = vsnprintf(&buffer[offset], maxSafeSize - offset, format, args);
	va_end (args);

	if(write == -1)
	{
		buffer[maxSafeSize] = 0;
		write = 0;
	}
#ifdef _DEBUG
	if(write > maxSafeSize)
		g_Log.AddMessageFormatW(MSG_WARN, "[STRING] SafeFormatOffset() cannot fit string [%s]", format);
#endif

	return write;
}

int ClipInt(int value, int min, int max)
{
	if(value < min)
		return min;
	if(value > max)
		return max;

	return value;
}

int ClipIntMin(int value, int min)
{
	if(value < min)
		return min;
	return value;
}

float ClipFloat(float value, float min, float max)
{
	if(value < min)
		return min;
	if(value > max)
		return max;

	return value;
}

//Rounds in the following manner: [-1.0 = -1] [-0.5 = -1] [0.5 = 0] [1.0 = 1]
float Round(float value)
{
	return ((value > 0.0F) ? floor(value + 0.5F) : ceil(value - 0.5F));
}

int QuaternionToByteFacing(double X, double Y, double Z, double W)
{
	/*
	Approaching the subject with zero knowledge of quaternions, I searched the
	internet on how to convert quaternions to angles (apparently referred to as
	Euler angles).  This code was adapted from this source material:

	https://irrlicht.svn.sourceforge.net/svnroot/irrlicht/trunk/include/quaternion.h

	Under the function quaternion::toEuler()

	*/

	double sqw = W*W;
	double sqx = X*X;
	double sqy = Y*Y;
	double sqz = Z*Z;
	double test = 2.0 * (Y*W - X*Z);

	double eulerX, eulerY; //, eulerZ;

	if (fabs(test - 1.0) <= 0.000001)
	{
		//eulerZ = (-2.0*atan2(X, W));
		eulerX = 0;
		eulerY = (PI/2.0);
	}
	else if (fabs(test - (-1.0)) <= 0.000001)
	{
		//eulerZ = (2.0*atan2(X, W));
		eulerX = 0;
		eulerY = (PI/-2.0);
	}
	else
	{
		//eulerZ = atan2(2.0 * (X*Y +Z*W),(sqx - sqy - sqz + sqw));
		eulerX = atan2(2.0 * (Y*Z +X*W),(-sqx - sqy + sqz + sqw));
		if(test < -1.0)
			test = -1.0;
		else if(test > 1.0)
			test = 1.0;
		eulerY = asin(test);
	}

	/* If looking at a circle:
	eulerX is zero for the right half
	eulerX is nonzero for the left half (positive or negative PI)
	eulerY is positive for the top half (values mirrored left/right)
	eulerY is negative for the bottom half (values mirrored left/right)
	*/
	
	/* THE FOLLOWING CONVERTS TO DEGREE ANGLES, AS REPORTED BY THE CLIENT
	   angle = 360 - ((eulerY / DOUBLE_PI) * 360)
	 */
	//East hemisphere
	if(eulerX == 0.0)
	{
		if(eulerY < 0)  //From 03:01 to 6:00 (-0.0 to -1.57)
			eulerY = (-eulerY);
		else            //From 12:00 to 2:59 (1.57 to 0.0)
			eulerY = DOUBLE_PI - eulerY;

	}
	else   //West hemisphere
	{
		if(eulerY < 0)  //From 6:00 to 8:59 (-1.57 to -0.0)
			eulerY = PI - (-eulerY);
		else            //From 9:01 to 12:00 (0.0 to 1.57)
			eulerY += PI;
	}

	//int degFacing = 360 - ((eulerY / DOUBLE_PI) * 360);
	
	/*Convert to byte facing (0-255).  For reference, going clockwise.
	  direction       byte     degrees
	   east (3:00)     64       360
	   south (6:00)    0[*]     270
	   west (9:00)     192      180
	   north (12:00)   128      90

       [*] : wraps around from 0 to 255.
     */
  
	int facing = 64 - (int)((eulerY / DOUBLE_PI) * 255.0);
	if(facing < 0)
		facing += 255;

	return facing;
}

//Wrapper for memset to be used for strings only, to help avoid careless uses of memset
//that could overwrite important information.
void ClearString(char *buffer, int size)
{
	memset(buffer, 0, size);
}

char* FormatTime(char *outBuf, int bufSize, int seconds)
{
	int Hour = seconds / 3600;
	int Day = Hour / 24;
	Hour -= Day * 24;
	int Minute = (seconds / 60) % 60;
	int Second = seconds % 60;

	if(Day == 0)
		Util::SafeFormat(outBuf, bufSize, "%02dh:%02dm:%02ds", Hour, Minute, Second);
	else
		Util::SafeFormat(outBuf, bufSize, "%02dd:%02dh:%02dm:%02ds", Day, Hour, Minute, Second);

	return outBuf;
}

void WriteIntegerList(FILE *output, const char* label, std::vector<int>& dataList)
{
	if(dataList.size() == 0)
		return;

	int write = 0;
	for(size_t i = 0; i < dataList.size(); i++)
	{
		if(write == 0)
			fprintf(output, "%s=", label);
		else if(write > 0)
			fputc(',', output);

		fprintf(output, "%d", dataList[i]);
		write++;
		if(write >= 10)
		{
			fprintf(output, "\r\n");
			write = 0;
		}
	}
	if(write > 0)
		fprintf(output, "\r\n");
}

bool DoubleEquivalent(double left, double right)
{
	return (fabs(left - right) <= 0.001);
}

bool FloatEquivalent(float left, float right)
{
	return (fabs(left - right) <= 0.001);
}

int GetAdditiveFromIntegralPercent100(int value, int multiplier)
{
	//Take a percentage multiplier expressed as an integer (100% = 100) 
	//and return the additive necessary to tally the final result.
	float retval = (float)value * ((float)multiplier / 100.0F);
	return (int)retval;
}

int GetAdditiveFromIntegralPercent1000(int value, int multiplier)
{
	//Take a percentage multiplier expressed as an integer (100% = 1000) 
	//and return the additive necessary to tally the final result.
	float retval = (float)value * ((float)multiplier / 1000.0F);
	return (int)retval;
}

int GetAdditiveFromIntegralPercent10000(int value, int multiplier)
{
	//Take a percentage multiplier expressed as an integer (100% = 10000) 
	//and return the additive necessary to tally the final result.
	float retval = (float)value * ((float)multiplier / 10000.0F);
	return (int)retval;
}

bool IntToBool(int value)
{
	return (value != 0);
}

//Replace nonprintable client characters.
void SanitizeClientString(char *string)
{
	for(size_t i = 0; i < strlen(string); i++)
		if(string[i] < 32 || string[i] >= 127)
			string[i] = '.';
}

void EncodeJSONString(std::string& str)
{
	ReplaceAll(str, "\n", "\\n");
	ReplaceAll(str, "\r", "\\r");
	ReplaceAll(str, "\"", "\\\"");
}

void ReplaceAll(std::string& str, const std::string& from, const std::string& to)
{
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos)
    {
		 str.replace(start_pos, from.length(), to);
		 start_pos += to.length();
    }
}

void RemoveStringsFrom(const char *search, std::string& operativeString)
{
	size_t pos = 0;
	size_t len = strlen(search);
	do
	{
		pos = operativeString.find(search, pos);
		if(pos != std::string::npos)
		{
			operativeString.erase(pos, len);
		}
	} while(pos != std::string::npos);
}

void ToLowerCase(std::string &input)
{
	for(size_t i = 0; i < input.size(); i++)
	{
		if(input[i] >= 'A' && input[i] <= 'Z')
			input[i] += 32;
	}
}

//Remove whitespace before or after a string.
void TrimWhitespace(std::string &modify)
{
	const char search[] = " \t";
	size_t pos = modify.find_first_not_of(search);
	if(pos != std::string::npos)
		modify.erase(0, pos);
	
	pos = modify.find_last_not_of(search);
	if(pos != std::string::npos)
		modify.erase(pos + 1, modify.length());
}

bool HasBeginning (std::string const &fullString, std::string const &beginning) {
	return fullString.size() >= beginning.size() ? fullString.substr(0, beginning.size()) == beginning : false;
}

bool HasEnding (std::string const &fullString, std::string const &ending) {
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}

float StringToFloat(const std::string &str)
{
	return static_cast<float>(atof(str.c_str()));
}

std::string FormatDate(time_t *time)
{
	char buff[20];
	struct tm *timeinfo = localtime(time);
	strftime(buff, 20, "%d/%m/%Y", timeinfo);
	return buff;
}

int ParseDate(const std::string &str, time_t &time)
{
	char buf[128];
	struct tm ptime;
	if(sscanf(str.c_str(), "%2d/%2d/%4d",
		&ptime.tm_mday,
		&ptime.tm_mon,
		&ptime.tm_year) != 3)
		return 0;
	time = mktime(&ptime);
	return 1;
}

int GetInteger(const STRINGLIST &strList, size_t index)
{
	if(index >= strList.size())
		return 0;
	return atoi(strList[index].c_str());
}

int GetInteger(const std::string &str)
{
	return atoi(str.c_str());
}

float GetFloat(const char *value)
{
	if(value == NULL)
		return 0.0F;
	return static_cast<float>(atof(value));
}

float GetFloat(const STRINGLIST &strList, size_t index)
{
	if(index >= strList.size())
		return 0.0F;
	return static_cast<float>(atof(strList[index].c_str()));
}

float GetFloat(const std::string &str)
{
	return static_cast<float>(atof(str.c_str()));
}

const char *GetString(const STRINGLIST &strList, size_t index)
{
	if(index >= strList.size())
		return NULL;
	return strList[index].c_str();
}

const char *GetSafeString(const STRINGLIST &strList, size_t index)
{
	if(index >= strList.size())
		return "";
	return strList[index].c_str();
}


//Break a comma-delimited string into tokens, then loading the converted values into a raw array of floats.
void AssignFloatArrayFromStringSplit(float *arrayDest, size_t arraySize, const std::string &strData)
{
	STRINGLIST strArray;
	Split(strData, ",", strArray);
	for(size_t i = 0; i < strArray.size(); i++)
	{
		if(i >= arraySize)
			return;
		arrayDest[i] = static_cast<float>(atof(strArray[i].c_str()));
	}
}

// Tokenize a string using whitespace as separators, but including string quotations.
void TokenizeByWhitespace(const std::string &input, STRINGLIST &output)
{
	std::string str = input;
	output.clear();
	size_t len = str.size();
	int first = -1;
	int last = -1;
	bool quote = false;
	bool terminate = false;
	for(size_t i = 0; i < len; i++)
	{
		switch(input[i])
		{
		case '"':
			terminate = true;  //Treat opening quote as a block start
			quote = !quote;
			break;

		case ' ':
		case '\t':
		case '\n':
		case '\r':
			if(quote == false)
				terminate = true;
			break;
		default:
			if(first == -1)
				first = i;
			last = i;
		}
		if(terminate == true)
		{
			if(first >= 0 && last >= 0)
			{
				std::string t = str.substr(first, last - first + 1);
				output.push_back(str.substr(first, last - first + 1));
				first = -1;
				last = -1;
			}
			terminate = false;
		}
	}
	if(first != -1 && first < (int)len)
		output.push_back(str.substr(first, len - first + 1));
}

std::string RandomStr()
{
//	static const char alphanum[] =
//		"0123456789"
//		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
//		"abcdefghijklmnopqrstuvwxyz";

	static const char alphanum[] =
			"23456789"
			"ABCDEFGHIJKLMNPQRSTUVWXYZ"
			"abcdefghjkmnpqrstuvwxyz";

	char str[32];
	for (unsigned int i = 0; i < sizeof(str); ++i) {
		str[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
	}
	str[sizeof(str) - 1] = 0;
	return str;
}


} //namespace Util
