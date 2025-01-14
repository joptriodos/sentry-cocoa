#import <Foundation/Foundation.h>

#import "SentryDefaultRateLimits.h"
#import "SentryDispatchQueueWrapper.h"
#import "SentryEnvelopeRateLimit.h"
#import "SentryHttpDateParser.h"
#import "SentryHttpTransport.h"
#import "SentryOptions.h"
#import "SentryQueueableRequestManager.h"
#import "SentryRateLimitParser.h"
#import "SentryRateLimits.h"
#import "SentryRetryAfterHeaderParser.h"
#import "SentryTransport.h"
#import "SentryTransportFactory.h"

NS_ASSUME_NONNULL_BEGIN

@interface
SentryTransportFactory ()

@end

@implementation SentryTransportFactory

+ (id<SentryTransport>)initTransport:(SentryOptions *)options
                   sentryFileManager:(SentryFileManager *)sentryFileManager
{
    NSURLSessionConfiguration *configuration =
        [NSURLSessionConfiguration ephemeralSessionConfiguration];
    NSURLSession *session = [NSURLSession sessionWithConfiguration:configuration delegate:options.urlSessionDelegate delegateQueue:nil];
    id<SentryRequestManager> requestManager =
        [[SentryQueueableRequestManager alloc] initWithSession:session];

    SentryHttpDateParser *httpDateParser = [[SentryHttpDateParser alloc] init];
    SentryRetryAfterHeaderParser *retryAfterHeaderParser =
        [[SentryRetryAfterHeaderParser alloc] initWithHttpDateParser:httpDateParser];
    SentryRateLimitParser *rateLimitParser = [[SentryRateLimitParser alloc] init];
    id<SentryRateLimits> rateLimits =
        [[SentryDefaultRateLimits alloc] initWithRetryAfterHeaderParser:retryAfterHeaderParser
                                                     andRateLimitParser:rateLimitParser];

    SentryEnvelopeRateLimit *envelopeRateLimit =
        [[SentryEnvelopeRateLimit alloc] initWithRateLimits:rateLimits];

    SentryDispatchQueueWrapper *dispatchQueueWrapper = [[SentryDispatchQueueWrapper alloc] init];

    return [[SentryHttpTransport alloc] initWithOptions:options
                                            fileManager:sentryFileManager
                                         requestManager:requestManager
                                             rateLimits:rateLimits
                                      envelopeRateLimit:envelopeRateLimit
                                   dispatchQueueWrapper:dispatchQueueWrapper];
}

@end

NS_ASSUME_NONNULL_END
