/*
 * $Id: tracker-client.c,v 1.2 2009/09/02 21:11:45 bruno Exp $
 *
 * @COPYRIGHT@
 * @COPYRIGHT@
 *
 * $Log: tracker-client.c,v $
 * Revision 1.2  2009/09/02 21:11:45  bruno
 * save the file locally. if it exists, then don't ask for it over the
 * network, just stream the file from the local disk
 *
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
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

#ifdef	LATER
	if ((status >= HTTP_OK) && (status <= HTTP_MULTI_STATUS)) {
		fwrite(ptr, size, nmemb, stdout);
	}
#endif

#ifdef	DEBUG
	logdebug("doheaders : ");
	logdebug(ptr);
#endif
	
	return(size * nmemb);
}

size_t
dobody(void *ptr, size_t size, size_t nmemb, void *stream)
{
	if ((status >= HTTP_OK) && (status <= HTTP_MULTI_STATUS)) {
		fwrite(ptr, size, nmemb, stream);
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
makeurl(char *header, char *filename, char *serverip, char *url,
	int url_max_size)
{
	int	length;

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
outputfile(char *filename, char *range)
{
	struct stat	statbuf;
	off_t		offset;
	size_t		lastbyte;
	size_t		totalbytes;
	size_t		bytesread;
	size_t		count;
	int		fd;
	char		buf[128*1024];
	char		done;

	/*
	 * make sure the file exists
	 */
	if (stat(filename, &statbuf) != 0) {
		return(-1);
	}

	/*
	 * if a range is supplied, then we need to calculate the offset
	 * and total number of bytes to read
	 */
	if (range != NULL) {

		/*
		 * there are three cases:
		 *
		 *	1) there is no 'offset' supplied. this means read from
		 *	   the beginning of the file (offset 0).
		 *
		 *	2) there is no 'last byte' supplied. this means read
		 *	   to the end of the file
		 *
		 *	3) both 'offset and 'last byte' are supplied.
		 */
		if (range[0] == '-') {
			/*
			 * case 1
			 */
			sscanf(range, "-%d", &lastbyte);
			offset = 0;
		} else if (range[strlen(range) - 1] == '-') {
			/*
			 * case 2
			 */
			sscanf(range, "%d-", &offset);
			lastbyte = statbuf.st_size;
		} else {
			/*
			 * case 3
			 */
			sscanf(range, "%d-%d", &offset, &lastbyte);
		}

		totalbytes = (lastbyte - offset) + 1;

	} else {
		offset = 0;
		totalbytes = statbuf.st_size;
	}

	if ((fd = open(filename, O_RDONLY)) < 0) {
		return(-1);
	}

	if (lseek(fd, offset, SEEK_SET) < 0) {
		return(-1);
	}

	/*
	 * output the HTTP headers
	 */
	if (range != NULL) {
		printf("HTTP/1.1 %d\n", HTTP_PARTIAL_CONTENT);
	} else {
		printf("HTTP/1.1 %d\n", status);
	}
	printf("Content-Type: application/octet-stream\n");
	printf("Content-Length: %d\n", totalbytes);
	printf("\n");

	bytesread = 0;
	done = 0;
	while (!done) {
		ssize_t	i;

		if ((sizeof(buf) + bytesread) > totalbytes) {
			count = totalbytes - bytesread;
		} else {
			count = sizeof(buf);
		}

		if ((i = read(fd, buf, count)) < 0) {
			/*
			 * log an error
			 */
			done = 1;
			continue;
		}

		/*
		 * output the buffer on stdout
		 */
		fwrite(buf, i, 1, stdout);

		bytesread += i;

		if (bytesread >= totalbytes) {
			done = 1;
		}
	}

	close(fd);
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

#ifdef	DEBUG
	logdebug("URL : ");
	logdebug(url);
	logdebug("\n");
#endif

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
		logdebug("\n");
#endif
	}

	if ((curlcode = curl_easy_perform(curlhandle)) != CURLE_OK) {
		senderror(500, "downloadfile:curl_easy_perform():failed",
			curlcode);
		return(-1);
	}

	return(0);
}

int
createdir(char *path)
{
	struct stat	buf;
	char		*ptr;
	char		*lastptr;
	char		done = 0;

	if (strlen(path) == 0)
		return(0);

	if (path[0] != '/') {
		return(0);
	}

	lastptr = &path[1];

	while (!done) {
		if ((ptr = index(lastptr, '/')) == NULL) {
			done = 1;
			continue;
		}

		*ptr = '\0';

		if (stat(path, &buf) != 0) {
			if (mkdir(path, 0755) != 0) {
				return(-1);
			}
		}

		*ptr = '/';

		lastptr = ptr + 1;
		if (lastptr >= (path + strlen(path))) {
			done = 1;
		}
	}

	/*
	 * we've created all the parent directories, now create the
	 * last directory
	 */
	if (mkdir(path, 0755) != 0) {
		return(-1);
	}

	return(0);
}

int
getfile(char *filename, char *serverip, char *range)
{
	CURL		*curlhandle;
	CURLoption	curloption;
	CURLcode	curlcode;
	struct stat	buf;
	int		curlparameter;
	char		url[PATH_MAX];

	status = HTTP_OK;

	/*
	 * check if the file is local
	 */
	if (stat(filename, &buf) != 0) {
		/*
		 * the file isn't on the local disk
		 */
		FILE	*file;
		char	*dir;
		char	*ptr;

		/*
		 * make sure the destination directory exists
		 */
		if ((dir = strdup(filename)) != NULL) {
			if ((ptr = rindex(dir, '/')) != NULL) {
				*ptr = '\0';
				if (stat(dir, &buf) != 0) {
					createdir(dir);
				}
			}

			free(dir);
		}

		/*
		 * make a 'http://' url and get the file.
		 */
		if ((file = fopen(filename, "w")) == NULL) {
			senderror(500, "downloadfile:fopen():failed", errno);
			return(-1);
		}

		/*
		 * initialize curl
		 */
		if ((curlhandle = curl_easy_init()) == NULL) {
			return(-1);
		}

#ifdef	DEBUG
		curl_easy_setopt(curlhandle, CURLOPT_VERBOSE, 1);
#endif

		/*
		 * tell curl to save it to disk (save it to the file pointed
		 * to by 'file'
		 */
		if ((curlcode = curl_easy_setopt(curlhandle,
				CURLOPT_WRITEDATA, file)) != CURLE_OK) {
			senderror(500, "downloadfile:curl_easy_setopt():failed",
				curlcode);
			return(-1);
		}

		if ((curlcode = curl_easy_setopt(curlhandle,
				CURLOPT_HEADERFUNCTION, doheaders)) !=
				CURLE_OK) {
			senderror(500, "downloadfile:curl_easy_setopt():failed",
				curlcode);
			return(-1);
		}

		if ((curlcode = curl_easy_setopt(curlhandle,
				CURLOPT_WRITEFUNCTION, dobody)) != CURLE_OK) {
			senderror(500, "downloadfile:curl_easy_setopt():failed",
				curlcode);
			return(-1);
		}

		if (makeurl("http://", filename, serverip, url,
				sizeof(url)) != 0) {
			senderror(500, "makeurl():failed", errno);
			return(0);
		}

		if (downloadfile(curlhandle, url, NULL) != 0) {
			senderror(500, "downloadfile():failed", 0);
		}

		fclose(file);

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

			/*
			 * on a failure, a zero-byte length file will be
			 * left on the disk -- this is because of the fopen().
			 * remove this zero-length file.
			 */
			unlink(filename);
		}

		/*
		 * cleanup curl
		 */
		curl_easy_cleanup(curlhandle);
	}

	/*
	 * now we know that the file is local, so read it and output it
	 * to stdout
	 */
	if ((status >= HTTP_OK) && (status <= HTTP_MULTI_STATUS)) {
		if (outputfile(filename, range) != 0) {
			senderror(500, "outputfile():failed", errno);
			return(0);
		}
	} else {
		printf("HTTP/1.1 %d\n", status);
		printf("\n");
	}

	return(0);
}

int
main()
{
	int		filenamelen;
	char		*msg = NULL; 
	char		*forminfo;
	char		*range;
	char		filename[PATH_MAX];
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

	if (getfile(filename, serverip, range) != 0) {
		senderror(500, "getfile():failed", errno);
		return(0);
	}
	
	return(0);
}

