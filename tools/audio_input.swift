import Foundation
import CoreAudio

func property(_ selector: AudioObjectPropertySelector, scope: AudioObjectPropertyScope = kAudioObjectPropertyScopeGlobal) -> AudioObjectPropertyAddress {
    AudioObjectPropertyAddress(mSelector: selector, mScope: scope, mElement: kAudioObjectPropertyElementMain)
}
func name(_ device: AudioDeviceID) -> String {
    var address = property(kAudioObjectPropertyName)
    var value: Unmanaged<CFString>?
    var size = UInt32(MemoryLayout<Unmanaged<CFString>?>.size)
    guard AudioObjectGetPropertyData(device, &address, 0, nil, &size, &value) == noErr else { return "unknown" }
    return value?.takeUnretainedValue() as String? ?? "unknown"
}
func inputDevice(_ device: AudioDeviceID) -> Bool {
    var address = property(kAudioDevicePropertyStreams, scope: kAudioDevicePropertyScopeInput)
    var size: UInt32 = 0
    return AudioObjectGetPropertyDataSize(device, &address, 0, nil, &size) == noErr && size > 0
}
var address = property(kAudioHardwarePropertyDevices)
var size: UInt32 = 0
let system = AudioObjectID(kAudioObjectSystemObject)
guard AudioObjectGetPropertyDataSize(system, &address, 0, nil, &size) == noErr else { exit(1) }
var devices = [AudioDeviceID](repeating: 0, count: Int(size)/MemoryLayout<AudioDeviceID>.size)
guard AudioObjectGetPropertyData(system, &address, 0, nil, &size, &devices) == noErr else { exit(1) }
var input = property(kAudioHardwarePropertyDefaultInputDevice)
var current: AudioDeviceID = 0
var inputSize = UInt32(MemoryLayout<AudioDeviceID>.size)
guard AudioObjectGetPropertyData(system, &input, 0, nil, &inputSize, &current) == noErr else { exit(1) }
print("DEFAULT_INPUT \(current) \(name(current))")
for device in devices where inputDevice(device) { print("INPUT \(device) \(name(device))") }
if CommandLine.arguments.contains("--select-sticks3") {
    guard let target = devices.first(where: { name($0) == "StickS3 Microphone" && inputDevice($0) }) else {
        fputs("StickS3 Microphone input not present\n", stderr); exit(2)
    }
    var selected = target
    guard AudioObjectSetPropertyData(system, &input, 0, nil, inputSize, &selected) == noErr else { exit(3) }
    guard AudioObjectGetPropertyData(system, &input, 0, nil, &inputSize, &current) == noErr, current == target else { exit(4) }
    print("SELECTED \(current) \(name(current))")
}
