#import "SentryDefines.h"
#import "SentryIntegrationProtocol.h"
#import "SentrySpanProtocol.h"

@class SentryEvent, SentryClient, SentryScope, SentrySession, SentryUser, SentryBreadcrumb,
    SentryId, SentryUserFeedback, SentryEnvelope, SentryTransactionContext;

NS_ASSUME_NONNULL_BEGIN
@interface SentryHub : NSObject
SENTRY_NO_INIT

- (instancetype)initWithClient:(SentryClient *_Nullable)client
                      andScope:(SentryScope *_Nullable)scope;

// Since there's no scope stack, single hub instance, experimenting with holding
// session here.
@property (nonatomic, readonly, strong) SentrySession *_Nullable session;

- (void)startSession;
- (void)endSessionWithTimestamp:(NSDate *)timestamp;
- (void)closeCachedSessionWithTimestamp:(NSDate *_Nullable)timestamp;

@property (nonatomic, strong)
    NSMutableArray<NSObject<SentryIntegrationProtocol> *> *installedIntegrations;

/**
 * Captures a manually created event and sends it to Sentry.
 *
 * @param event The event to send to Sentry.
 *
 * @return The SentryId of the event or SentryId.empty if the event is not sent.
 */
- (SentryId *)captureEvent:(SentryEvent *)event NS_SWIFT_NAME(capture(event:));

/**
 * Captures a manually created event and sends it to Sentry.
 *
 * @param event The event to send to Sentry.
 * @param scope The scope containing event metadata.
 *
 * @return The SentryId of the event or SentryId.empty if the event is not sent.
 */
- (SentryId *)captureEvent:(SentryEvent *)event
                 withScope:(SentryScope *)scope NS_SWIFT_NAME(capture(event:scope:));

/**
 * Creates a transaction bound to the hub and returns the instance.
 *
 * @param name The transaction name.
 * @param operation Short code identifying the type of operation the span is measuring.
 *
 * @return The created transaction.
 */
- (id<SentrySpan>)startTransactionWithName:(NSString *)name
                                 operation:(NSString *)operation
    NS_SWIFT_NAME(startTransaction(name:operation:));

/**
 * Creates a transaction bound to the hub and returns the instance.
 *
 * @param transactionContext The transaction context.
 *
 * @return The created transaction.
 */
- (id<SentrySpan>)startTransactionWithContext:(SentryTransactionContext *)transactionContext
    NS_SWIFT_NAME(startTransaction(transactionContext:));

/**
 * Creates a transaction bound to the hub and returns the instance.
 *
 * @param transactionContext The transaction context.
 * @param customSamplingContext Additional information about the sampling context.
 *
 * @return The created transaction.
 */
- (id<SentrySpan>)startTransactionWithContext:(SentryTransactionContext *)transactionContext
                        customSamplingContext:
                            (nullable NSDictionary<NSString *, id> *)customSamplingContext
    NS_SWIFT_NAME(startTransaction(transactionContext:customSamplingContext:));

/**
 * Captures an error event and sends it to Sentry.
 *
 * @param error The error to send to Sentry.
 *
 * @return The SentryId of the event or SentryId.empty if the event is not sent.
 */
- (SentryId *)captureError:(NSError *)error NS_SWIFT_NAME(capture(error:));

/**
 * Captures an error event and sends it to Sentry.
 *
 * @param error The error to send to Sentry.
 * @param scope The scope containing event metadata.
 *
 * @return The SentryId of the event or SentryId.empty if the event is not sent.
 */
- (SentryId *)captureError:(NSError *)error
                 withScope:(SentryScope *)scope NS_SWIFT_NAME(capture(error:scope:));

/**
 * Captures an exception event and sends it to Sentry.
 *
 * @param exception The exception to send to Sentry.
 *
 * @return The SentryId of the event or SentryId.empty if the event is not sent.
 */
- (SentryId *)captureException:(NSException *)exception NS_SWIFT_NAME(capture(exception:));

/**
 * Captures an exception event and sends it to Sentry.
 *
 * @param exception The exception to send to Sentry.
 * @param scope The scope containing event metadata.
 *
 * @return The SentryId of the event or SentryId.empty if the event is not sent.
 */
- (SentryId *)captureException:(NSException *)exception
                     withScope:(SentryScope *)scope NS_SWIFT_NAME(capture(exception:scope:));

/**
 * Captures a message event and sends it to Sentry.
 *
 * @param message The message to send to Sentry.
 *
 * @return The SentryId of the event or SentryId.empty if the event is not sent.
 */
- (SentryId *)captureMessage:(NSString *)message NS_SWIFT_NAME(capture(message:));

/**
 * Captures a message event and sends it to Sentry.
 *
 * @param message The message to send to Sentry.
 * @param scope The scope containing event metadata.
 *
 * @return The SentryId of the event or SentryId.empty if the event is not sent.
 */
- (SentryId *)captureMessage:(NSString *)message
                   withScope:(SentryScope *)scope NS_SWIFT_NAME(capture(message:scope:));

/**
 * Captures a manually created user feedback and sends it to Sentry.
 *
 * @param userFeedback The user feedback to send to Sentry.
 */
- (void)captureUserFeedback:(SentryUserFeedback *)userFeedback
    NS_SWIFT_NAME(capture(userFeedback:));

/**
 * Invokes the callback with a mutable reference to the scope for modifications.
 */
- (void)configureScope:(void (^)(SentryScope *scope))callback;

/**
 * Adds a breadcrumb to the current scope.
 */
- (void)addBreadcrumb:(SentryBreadcrumb *)crumb;

/**
 * Returns a client if there is a bound client on the Hub.
 */
- (SentryClient *_Nullable)getClient;

/**
 * Returns either the current scope and if nil a new one.
 */
- (SentryScope *)getScope __deprecated_msg("Use SentryHub.scope instead.");

/**
 * Returns either the current scope and if nil a new one.
 */
@property (nonatomic, readonly, strong) SentryScope *scope;

/**
 * Binds a different client to the hub.
 */
- (void)bindClient:(SentryClient *_Nullable)client;

/**
 * Checks if integration is activated for bound client and returns it.
 */
- (id _Nullable)getIntegration:(NSString *)integrationName;

/**
 * Checks if a specific Integration (`integrationClass`) has been installed.
 * @return BOOL If instance of `integrationClass` exists within
 * `SentryHub.installedIntegrations`.
 */
- (BOOL)isIntegrationInstalled:(Class)integrationClass;

/**
 * Set global user -> thus will be sent with every event
 */
- (void)setUser:(SentryUser *_Nullable)user;

/**
 * The SDK reserves this method for hybrid SDKs, which use it to capture events.
 *
 * @discussion We increase the session error count if an envelope is passed in containing an
 * event with event.level error or higher. Ideally, we would check the mechanism and/or exception
 * list, like the Java and Python SDK do this, but this would require full deserialization of the
 * event.
 */
- (void)captureEnvelope:(SentryEnvelope *)envelope NS_SWIFT_NAME(capture(envelope:));

@end

NS_ASSUME_NONNULL_END
