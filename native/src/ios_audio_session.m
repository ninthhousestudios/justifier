// ios_audio_session.m — AVAudioSession helpers for iOS
// Compiled only on iOS (guarded in CMakeLists.txt).

#import <AVFoundation/AVFoundation.h>

void justifier_ios_set_session_play_and_record(void) {
    NSError *error = nil;
    AVAudioSession *session = [AVAudioSession sharedInstance];
    [session setCategory:AVAudioSessionCategoryPlayAndRecord
             withOptions:AVAudioSessionCategoryOptionDefaultToSpeaker |
                         AVAudioSessionCategoryOptionAllowBluetooth
                   error:&error];
    if (error) {
        NSLog(@"justifier: failed to set PlayAndRecord: %@", error);
    }
    [session setActive:YES error:&error];
}

void justifier_ios_set_session_playback(void) {
    NSError *error = nil;
    AVAudioSession *session = [AVAudioSession sharedInstance];
    [session setCategory:AVAudioSessionCategoryPlayback error:&error];
    if (error) {
        NSLog(@"justifier: failed to set Playback: %@", error);
    }
}
