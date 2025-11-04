/*
 * Copyright 2025 Raphael Mudge, Adversary Fan Fiction Writers Guild
 *
 * Redistribution and use in source and binary forms, with or without modification, are
 * permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of
 * conditions and the following disclaimer in the documentation and/or other materials provided
 * with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used to
 * endorse or promote products derived from this software without specific prior written
 * permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS IS” AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <windows.h>
#include "tcg.h"
#include "HTTP.h"

WINBASEAPI void MSVCRT$free(void *ptr);
WINBASEAPI size_t MSVCRT$strlen(const char *str);
WINBASEAPI LPVOID WINAPI KERNEL32$VirtualAlloc (LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect);
WINBASEAPI errno_t MSVCRT$strcpy_s(char *strDestination, size_t numberOfElements, const char *strSource);

/**
 * Dynamic Function Resolution resolver.
 * Turns MODULE$Function into pointers.
 * See dfr "resolve" in loader.spec.
 */
char * resolve(DWORD modHash, DWORD funcHash) {
	char * hModule = (char *)findModuleByHash(modHash);
	return findFunctionByHash(hModule, funcHash);
}

/**
 * Helper function to fix pointers in x86 PIC.
 * See fixptrs _caller in loader.spec.
 */
#ifdef WIN_X86
__declspec(noinline) ULONG_PTR caller( VOID ) { return (ULONG_PTR)WIN_GET_CALLER(); }
#endif

/**
 * Crystal Palace convention for accessing data linked with this loader.
 */
char __BOFDATA__[0] __attribute__((section("my_data")));

char * findAppendedPICO() {
	return (char *)&__BOFDATA__;
}

/**
 * PICO loader entry point.
 */
void go() {
	char        * dstCode;
	char        * dstData;
	char        * src;
	IMPORTFUNCS   funcs;

	/* Find the DLL appended to this PIC. */
	src = findAppendedPICO();

	/* Allocate memory for the PICO code and data sections. */
	dstCode = KERNEL32$VirtualAlloc( NULL, PicoCodeSize(src), MEM_RESERVE|MEM_COMMIT|MEM_TOP_DOWN, PAGE_EXECUTE_READWRITE );
	dstData = KERNEL32$VirtualAlloc( NULL, PicoDataSize(src), MEM_RESERVE|MEM_COMMIT|MEM_TOP_DOWN, PAGE_READWRITE );

	/* Setup the IMPORTFUNCS data structure. */
	funcs.GetProcAddress = GetProcAddress;
	funcs.LoadLibraryA   = LoadLibraryA;

	/* Load the PICO into destination addresses. */
	PicoLoad(&funcs, src, dstCode, dstData);

	/**
	 * LibHttp usage example demonstrating multiple HTTP operations.
	 */
	{
		dprintf("[*] LibHttp Example: Initializing HTTP client\n");

		/* Load WinHTTP library for HTTP operations. */
		LoadLibraryA("WINHTTP");

		/* Initialize HTTP client with HTTPS support. */
		HttpHandle *http_client = HttpInit(1);
		if (!http_client) {
			dprintf("[-] Failed to initialize HTTP client\n");
			goto pico_entry;
		}
		dprintf("[+] HTTP client initialized successfully\n");

		/* Configure client timeout and User-Agent. */
		http_client->response_timeout_ms = 10000;
		MSVCRT$strcpy_s(http_client->user_agent, sizeof(http_client->user_agent), "MyApp/2.0");
		dprintf("[+] Client configured: timeout=%ldms, user_agent=MyApp/2.0\n", http_client->response_timeout_ms);

		/* Example 1: Simple GET request. */
		dprintf("[*] Example 1: Sending GET request to httpbin.org/get\n");
		HttpURI uri = {"httpbin.org", 443, "/get"};
		HttpResponse response = {0};
		BOOL ok = HttpRequest(
			http_client,
			HTTP_METHOD_GET,
			&uri,
			NULL,
			NULL,
			&response
		);
		if (ok) {
			dprintf("[+] GET request successful - Status: %d, Body size: %zu bytes\n", response.status_code, response.body_size);
			MSVCRT$free(response.body);
			MSVCRT$free(response.content_type);
		} else {
			dprintf("[-] GET request failed\n");
		}

		/* Example 2: POST request with JSON body and custom headers. */
		dprintf("[*] Example 2: Sending POST request with JSON body\n");
		HttpHeader headers_array[1];
		headers_array[0].name = "Content-Type";
		headers_array[0].value = "application/json";
		HttpHeaders headers = {headers_array, 1};
		const char *json_body = "{\"message\":\"Hello World\"}";
		HttpBody body = {(const BYTE *)json_body, MSVCRT$strlen(json_body)};
		uri.port = 443;
		uri.path = "/post";
		response = (HttpResponse){0};
		ok = HttpRequest(
			http_client,
			HTTP_METHOD_POST,
			&uri,
			&headers,
			&body,
			&response
		);
		if (ok) {
			dprintf("[+] POST request successful - Status: %d, Body size: %zu bytes\n", response.status_code, response.body_size);
			MSVCRT$free(response.body);
			MSVCRT$free(response.content_type);
		} else {
			dprintf("[-] POST request failed\n");
		}

		/* Example 3: Error handling with invalid host. */
		dprintf("[*] Example 3: Testing error handling with invalid host\n");
		uri.host = "doesnotexist.invalid";
		uri.path = "/";
		response = (HttpResponse){0};
		ok = HttpRequest(
			http_client,
			HTTP_METHOD_GET,
			&uri,
			NULL,
			NULL,
			&response
		);
		if (!ok) {
			dprintf("[!] Expected error occurred - request to invalid host failed as expected\n");
		}

		/* Cleanup: destroy the HTTP client. */
		dprintf("[*] Cleaning up - destroying HTTP client\n");
		HttpDestroy(http_client);
		dprintf("[+] HTTP client destroyed successfully\n");

		goto pico_entry;
	}

pico_entry:
	/* Execute the PICO code. */
	PicoEntryPoint(src, dstCode) (NULL);
}