#ifndef H_LOADER_URLS
#define H_LOADER_URLS

#ifdef ROCKS
enum urlprotocol_t { URL_METHOD_FTP, URL_METHOD_HTTP, URL_METHOD_HTTPS };
#else
enum urlprotocol_t { URL_METHOD_FTP, URL_METHOD_HTTP };
#endif
typedef enum urlprotocol_t urlprotocol;

struct iurlinfo {
    urlprotocol protocol;
    char * address;
    char * login;
    char * password;
    char * prefix;
    char * proxy;
    char * proxyPort;
    int ftpPort;
};

int convertURLToUI(char *url, struct iurlinfo *ui);
char *convertUIToURL(struct iurlinfo *ui);

int setupRemote(struct iurlinfo * ui);
int urlMainSetupPanel(struct iurlinfo * ui, urlprotocol protocol,
		      char * doSecondarySetup);
int urlSecondarySetupPanel(struct iurlinfo * ui, urlprotocol protocol);
int urlinstStartTransfer(struct iurlinfo * ui, char * filename, 
                         char *extraHeaders);
int urlinstFinishTransfer(struct iurlinfo * ui, int fd);

#ifdef ROCKS
#include <openssl/bio.h>
BIO* urlinstStartSSLTransfer(struct iurlinfo * ui, char * filename,
			char *extraHeaders, int silentErrors, int flags);
int urlinstFinishSSLTransfer(BIO *sbio);

int haveCertificate();

#include "loader.h"
int getCert(struct loaderData_s *loaderData);
#endif

#endif
