// Copyright © 2019, Université de Lille.
// Created: 22 janvier 2019
// 
// Corresponding author: Michael Hauspie <michael.hauspie@univ-lille.fr>
// For full author list, see AUTHORS.md file
// 
// This software is governed by the CeCILL license under French law and
// abiding by the rules of distribution of free software.  You can  use,
// modify and/ or redistribute the software under the terms of the CeCILL
// license as circulated by CEA, CNRS and INRIA at the following URL
// "http://www.cecill.info".
// 
// As a counterpart to the access to the source code and  rights to copy,
// modify and redistribute granted by the license, users are provided only
// with a limited warranty  and the software author,  the holder of the
// economic rights,  and the successive licensors  have only  limited
// liability.
// 
// In this respect, the user attention is drawn to the risks associated
// with loading,  using,  modifying and/or developing or reproducing the
// software by the user in light of its specific status of free software,
// that may mean  that it is complicated to manipulate,  and  that  also
// therefore means  that it is reserved for developers  and  experienced
// professionals having in-depth computer knowledge. Users are therefore
// encouraged to load and test the software suitability as regards their
// requirements in conditions enabling the security of their systems and/or
// data to be ensured and,  more generally, to use and operate it in the
// same conditions as regards security.
// 
// The fact that you are presently reading this means that you have had
// knowledge of the CeCILL license and that you accept its terms.
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "http_parse.h"

/** find the minimum value between two numbers */
#define min(a, b) ((a) < (b) ? (a) : (b))

/** find if a number is in two others */
#define in_range(a, b, c) ((a) < (b) ? 0 : ((a) > (c) ? 0 : 1))

/**
 * function to parse the request
 * @param data
 * @param request request to parse
 * @return 1 on success, 0 on error
 */
int parse_http_request(char *request_line, char *headers_block, http_request *request) {
    request->header_count = 0;

    // --- 1. Parse la ligne de requête ---
    char *method = strtok(request_line, " ");
    char *target = strtok(NULL, " ");
    char *version = strtok(NULL, "\r\n");

    if (!method || !target || !version)
        return 0;

    // Méthode
    if (strcmp(method, "GET") == 0) {
        request->method = HTTP_GET;
    } else if (strcmp(method, "HEAD") == 0) {
        request->method = HTTP_HEAD;
    } else {
        request->method = HTTP_UNSUPPORTED;
    }

    // Target
    strncpy(request->target, target, sizeof(request->target) - 1);
    request->target[sizeof(request->target) - 1] = '\0';

    // HTTP version
    if (sscanf(version, "HTTP/%d.%d", &request->http_major, &request->http_minor) != 2)
        return 0;

    // --- 2. Parser les headers ligne par ligne ---
    char *line = headers_block;

    while (line && *line != '\0') {
        // Fin des headers
        if (strncmp(line, "\r\n", 2) == 0 || strncmp(line, "\n", 1) == 0)
            break;

        char *colon = strchr(line, ':');
        if (!colon)
            return 0;

        // Nom du header
        int name_len = colon - line;
        if (name_len >= MAX_HEADER_NAME_SIZE)
            return 0;

        char name[MAX_HEADER_NAME_SIZE];
        strncpy(name, line, name_len);
        name[name_len] = '\0';

        // Nettoyer le nom
        for (int i = 0; i < name_len; i++)
            name[i] = tolower(name[i]);

        // Sauter les espaces après le :
        char *value = colon + 1;
        while (*value == ' ' || *value == '\t') value++;

        int value_len = strcspn(value, "\r\n");
        if (value_len >= MAX_HEADER_VALUE_SIZE)
            return 0;

        char val[MAX_HEADER_VALUE_SIZE];
        strncpy(val, value, value_len);
        val[value_len] = '\0';

        // Stocker Host et User-Agent
        if (strcmp(name, "host") == 0) {
            strncpy(request->host_header, val, sizeof(request->host_header) - 1);
            request->host_header[sizeof(request->host_header) - 1] = '\0';
        } else if (strcmp(name, "user-agent") == 0) {
            strncpy(request->user_agent, val, sizeof(request->user_agent) - 1);
            request->user_agent[sizeof(request->user_agent) - 1] = '\0';
        }

        // Prochaine ligne
        line = strstr(line, "\n");
        if (line)
            line++;
    }

    return 1;
}

/**
 * get the string name of the http method enum
 * @param method the method enum to check
 * @return the text name of the method
 */
char *get_method(enum http_method method) {
    switch (method) {
        case HTTP_GET:
            return "GET";
        case HTTP_HEAD:
            return "HEAD";
        case HTTP_UNSUPPORTED:
            return "UNSUPPORTED";
    }
    return "UNSUPPORTED";
}

#ifdef COMPILE_MAIN
int main(int argc, char **argv) {
   if (argc != 2) {
      fprintf(stderr, "usage: %s http_request_line\n", argv[0]);
      return 1;
   }

   http_request r;
   if (!parse_http_request(argv[1], &r)) {
      fprintf(stderr, "Fails to parse request\n");
      if (r.method == HTTP_UNSUPPORTED)
         fprintf(stderr, "Unsupported method\n");
      return 1;
   }
   printf("request line: %s\n", argv[1]);
   printf("method: %s\n", r.method == HTTP_GET ? "GET" : "UNSUPPORTED");
   printf("target: %s\n", r.target);
   printf("version: %d.%d\n", r.http_major, r.http_minor);
   return 0;
}

#endif
