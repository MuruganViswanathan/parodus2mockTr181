/**
 * Copyright 2018 Comcast Cable Communications Management, LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include <hiredis/hiredis.h>

#include "mock_tr181_client.h"
#include "mock_tr181_adapter.h"

#define REDIS_HOSTADDR "127.0.0.1"
#define REDIS_HOSTPORT 6379
static redisContext *g_redisContext = NULL;


/*----------------------------------------------------------------------------*/
/*                 External interface functions                               */
/*----------------------------------------------------------------------------*/
/*
 * returns 0 - failure
 *         1 - success
 */
static int redis_tr181_db_init()
{
	if (g_redisContext == NULL)
	{
		struct timeval timeout = { 1, 500000 }; // 1.5 seconds
		g_redisContext = redisConnectWithTimeout(REDIS_HOSTADDR, REDIS_HOSTPORT, timeout);
	}

	if (g_redisContext == NULL || g_redisContext->err)
	{
		if (g_redisContext)
		{
			printf("Connection error: %s\n", g_redisContext->errstr);
			redisFree(g_redisContext);
			g_redisContext = NULL;
		}
		else
		{
			printf("Connection error: can't allocate redis context\n");
		}

		return 0;
	}

	return 1;
}


void connect_tr181_db()
{
	int count = 3;

	while (count--)
	{
		if (redis_tr181_db_init())
		{
			Info("Connection to Tr181 DB Success.\n");
			break;
		}
		else
		{
			Error("Connection to Tr181 DB failed! %s.\n", (count ? "Retrying again.." : "Quiting!"));
			sleep(5);
		}
	}
}



/*
 * Get param value for a given key from redis db
 * key - key string to search for
 * Returns value or Error as null terminated string.
 * NOTE: Caller should free return value char*
 */
char*  redis_tr181_param_get(char* key)
{
	char* value = NULL;

	if(g_redisContext == NULL)
	{
		Error("Not connected to Tr181 DB ! Attempting to connect...");
		connect_tr181_db();
	}

	if(g_redisContext)
	{
		char* req = NULL;
		if(key)
		{
			int l = strlen(key) + 5; //for GET Req
			req = (char*)malloc(sizeof(char)*l);
			if(req)
			{
				memset(req, 0, sizeof(char)*l);
				snprintf(req, sizeof(char)*l, "GET %s", key);
			}
		}

		if(req)
		{
			redisReply *reply = redisCommand(g_redisContext, req);
			printf("GET foo: %s\n", reply->str);
			if(reply->type == REDIS_REPLY_STRING)
			{
				value = strdup(reply->str);
			}
			freeReplyObject(reply);
		}
	}

	return value;
}


/*
 * Reads the entire ascii char file into char buffer
 * returns 0 - failure
 *         1 - success
 */
int readFromDB(char **data)
{
	FILE *fp;
	int ch_count = 0;
	fp = fopen(DB_FILE, "r");
	if (fp == NULL)
	{
		Error("Failed to open file %s\n", DB_FILE);
		return 0;
	}
	fseek(fp, 0, SEEK_END);  //set file position to end
	ch_count = ftell(fp);    // get the file position
	fseek(fp, 0, SEEK_SET);  //set file position to start

	*data = (char *) malloc(sizeof(char) * (ch_count + 1)); //allocate memory for file size
	if (*data != NULL)
	{
		fread(*data, 1, ch_count, fp);  // copy file to memory
		(*data)[ch_count] = '\0';
	}
	else
	{
		Error("Failed to allocate memory of %d bytes\n", ch_count);
		return 0;
	}

	fclose(fp);
	return 1;
}

/*
 * returns 0 - failure
 *         1 - success
 */
int writeToDB(char *data)
{
	FILE *fp;
	fp = fopen(DB_FILE, "w");
	if (fp == NULL)
	{
		Error("Failed to open file %s\n", DB_FILE);
		return 0;
	}
	fwrite(data, strlen(data), 1, fp);
	fclose(fp);
	return 1;
}
