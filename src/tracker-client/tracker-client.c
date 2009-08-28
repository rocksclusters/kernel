/*
 * $Id: tracker-client.c,v 1.1 2009/08/28 21:49:53 bruno Exp $
 *
 * @COPYRIGHT@
 * @COPYRIGHT@
 *
 * $Log: tracker-client.c,v $
 * Revision 1.1  2009/08/28 21:49:53  bruno
 * the start of the "most scalable installer in the universe!"
 *
 *
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <curl/curl.h>
#include <httpd/httpd.h>

int	status = HTTP_OK;


int
getargs(char *forminfo, char *filename, char *serverip)
{
	char	*ptr;
	int	length;

	/*
	 * help out sscanf by putting in a blank for '&'
	 */
	if ((ptr = strchr(forminfo, '&')) == NULL) {
		/*
		 * XXX - log an error
		 */
		return(-1);
	}

	*ptr = ' ';

	if (sscanf(forminfo, "filename=%4095s serverip=%15s", filename,
			serverip) != 2) {
		/*
		 * XXX - log an error
		 */
		return(-1);
	}

	return(0);
}

#ifdef	DEBUG
void
logdebug(char *msg)
{
	FILE	*file;

	if ((file = fopen("/tmp/tracker-client.debug", "a+")) != NULL) {
		fprintf(file, "%s", msg);
		fclose(file);
	}

	return;
}
#endif


size_t
doheaders(void *ptr, size_t size, size_t nmemb, void *stream)
{
	int	httpstatus;

	/*
	 * look for HTTP status code. if it is not 200, then set the status
	 * to -1 (this tells us not to output any data).
	 */
	if ((status >= HTTP_OK) && (status <= HTTP_MULTI_STATUS)) {
		if (sscanf(ptr, "HTTP/1.1 %d", &httpstatus) == 1) {
			status = httpstatus;
		}
	}

	if ((status >= HTTP_OK) && (status <= HTTP_MULTI_STATUS)) {
		fwrite(ptr, size, nmemb, stdout);
	}

#ifdef	DEBUG
	logdebug(ptr);
#endif
	
	return(size * nmemb);
}

size_t
dobody(void *ptr, size_t size, size_t nmemb, void *stream)
{
	if ((status >= HTTP_OK) && (status <= HTTP_MULTI_STATUS)) {
		fwrite(ptr, size, nmemb, stdout);
	}

	return(size * nmemb);
}

void
senderror(int http_error_code, char *msg, int other_error_code)
{
	/*
	 * HTTP/1.1 <errorcode> <error name>
	 *	
	 *	e.g., "HTTP/1.1 404 Not Found"
	 *	
	 */
        printf("HTTP/1.1 %d\n", http_error_code);

        printf("Content-type: text/plain\n");
        printf("Status: %d\n\n", http_error_code);
        printf("%s\n", msg);
        printf("other error code (%d)\n", other_error_code);
}

int
makeurl(char *filename, char *serverip, char *url, int url_max_size)
{
	int	length;
	char	*header = "http://";

	/*
	 * +2 is for null terminator and (potential) added '/' character
	 */
	length = strlen(header) + strlen(serverip) + strlen(filename) + 2;

	if (length > url_max_size) {
		return(-1);
	}

	sprintf(url, "%s%s", header, serverip);

	if (filename[0] != '/') {
		sprintf(url, "%s/", url);
	}

	sprintf(url, "%s%s", url, filename);

	return(0);
}

int
downloadfile(CURL *curlhandle, char *url, char *range)
{
	CURLcode	curlcode;

	if ((curlcode = curl_easy_setopt(curlhandle, CURLOPT_URL, url)) !=
			CURLE_OK) {
		senderror(500, "downloadfile:curl_easy_setopt():failed",
			curlcode);
		return(-1);
	}

	if ((curlcode = curl_easy_setopt(curlhandle, CURLOPT_HEADERFUNCTION,
			doheaders)) != CURLE_OK) {
		senderror(500, "downloadfile:curl_easy_setopt():failed",
			curlcode);
		return(-1);
	}

	if ((curlcode = curl_easy_setopt(curlhandle, CURLOPT_WRITEFUNCTION,
			dobody)) != CURLE_OK) {
		senderror(500, "downloadfile:curl_easy_setopt():failed",
			curlcode);
		return(-1);
	}

	if (range != NULL) {
		if ((curlcode = curl_easy_setopt(curlhandle, CURLOPT_RANGE,
				range)) != CURLE_OK) {
			senderror(500, "downloadfile:curl_easy_setopt():failed",
				curlcode);
			return(-1);
		}
#ifdef	DEBUG
		logdebug("HTTP RANGE: ");
		logdebug(range);
#endif
	}

	/*
	 * set the global status to 0. a non-zero global status tells that
	 * we got an http error code from one of our peers during the download
	 */
	status = HTTP_OK;

	if ((curlcode = curl_easy_perform(curlhandle)) != CURLE_OK) {
		senderror(500, "downloadfile:curl_easy_perform():failed",
			curlcode);
		return(-1);
	}

	return(0);
}

int
main()
{
	CURL		*curlhandle;
	CURLoption	curloption;
	int		curlparameter;
	int		filenamelen;
	char		*msg = NULL; 
	char		*forminfo;
	char		*range;
	char		filename[PATH_MAX];
	char		url[PATH_MAX];
	char		serverip[16];

	bzero(filename, sizeof(filename));
	bzero(serverip, sizeof(serverip));

	if ((forminfo = getenv("QUERY_STRING")) == NULL) {
		senderror(500, "No QUERY_STRING", errno);
		return(0);
	}

	if ((range = getenv("HTTP_RANGE")) != NULL) {
		char	*ptr;

		if ((ptr = strchr(range, '=')) != NULL) {
			range = ptr + 1;
		}
	}

	if (getargs(forminfo, filename, serverip) != 0) {
		senderror(500, "getargs():failed", errno);
		return(0);
	}
	
	if (makeurl(filename, serverip, url, sizeof(url)) != 0) {
		senderror(500, "makeurl():failed", errno);
		return(0);
	}

	/*
	 * initialize curl
	 */
	curlhandle = curl_easy_init();

#ifdef	DEBUG
	curl_easy_setopt(curlhandle, CURLOPT_VERBOSE, 1);
#endif

	if (curlhandle) {
		if (downloadfile(curlhandle, url, range) != 0) {
			senderror(500, "downloadfile():failed", 0);
		}

		if ((status < HTTP_OK) || (status > HTTP_MULTI_STATUS)) {
			/*
			 * send back an error status message
			 */
			char	*msgheader = "Couldn't download file ";
			char	*msg;
			int	len = strlen(msgheader) + strlen(filename) + 1;

			if ((msg = (char *)malloc(len)) != NULL) {
				sprintf(msg, "%s%s", msgheader, filename);
				senderror(status, msg, 0);

				free(msg);
			} else {
				senderror(status, "Couldn't download file", 0);
			}
			
		}

		/*
		 * cleanup curl
		 */
		curl_easy_cleanup(curlhandle);
	}

	return(0);
}

