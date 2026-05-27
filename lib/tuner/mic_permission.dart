import 'package:flutter/services.dart';

enum MicPermissionStatus { granted, denied, permanentlyDenied }

class MicPermission {
  static const _channel = MethodChannel('studio.ninthhouse.justifier/mic');

  static Future<MicPermissionStatus> request() async {
    final result = await _channel.invokeMethod<String>('request');
    return switch (result) {
      'granted' => MicPermissionStatus.granted,
      'permanentlyDenied' => MicPermissionStatus.permanentlyDenied,
      _ => MicPermissionStatus.denied,
    };
  }
}
