import Sentry
import UIKit

class FakeDelegateSwift: NSObject, URLSessionDelegate {

    func urlSession(_ session: URLSession, didReceive challenge: URLAuthenticationChallenge, completionHandler: @escaping (URLSession.AuthChallengeDisposition, URLCredential?) -> Void) {
        print("HATSEE 5!")
    }
}

@UIApplicationMain
class AppDelegate: UIResponder, UIApplicationDelegate {

    let fakeDelegate = FakeDelegateSwift()

    func application(_ application: UIApplication, didFinishLaunchingWithOptions launchOptions: [UIApplication.LaunchOptionsKey: Any]?) -> Bool {
        
        SentrySDK.start { options in
            options.dsn = "https://4a0f226d3bfb4a15867ddd1e36f0d4c0@o529868.ingest.sentry.io/5648995"
            options.beforeSend = { event in
                return event
            }
            options.debug = true
            options.sessionTrackingIntervalMillis = 5_000
            options.urlSessionDelegate = self.fakeDelegate
        }
        
        return true
    }

    // MARK: UISceneSession Lifecycle

    func application(_ application: UIApplication, configurationForConnecting connectingSceneSession: UISceneSession, options: UIScene.ConnectionOptions) -> UISceneConfiguration {
        // Called when a new scene session is being created.
        // Use this method to select a configuration to create the new scene with.
        return UISceneConfiguration(name: "Default Configuration", sessionRole: connectingSceneSession.role)
    }

    func application(_ application: UIApplication, didDiscardSceneSessions sceneSessions: Set<UISceneSession>) {
        // Called when the user discards a scene session.
        // If any sessions were discarded while the application was not running, this will be called shortly after application:didFinishLaunchingWithOptions.
        // Use this method to release any resources that were specific to the discarded scenes, as they will not return.
    }

}
