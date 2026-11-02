// Generated from a Firefox 159 headless measurement of interface members the
// engine had not installed (investigation §28 surface audit). Presence-only:
// functions and accessors answer undefined, constants carry the measured
// value; every definition is skipped when the name already exists, so a
// subsystem's real member always wins.
#include "runtime/surface_parity.h"
#include "js/PropertyAndElement.h"

namespace ruyijs {
namespace {
bool SurfaceUndefined(JSContext* cx, unsigned, JS::Value* vp) {
  JS::CallArgsFromVp(0, vp).rval().setUndefined();
  return true;
}
bool SurfaceIgnoreSet(JSContext* cx, unsigned, JS::Value* vp) {
  JS::CallArgsFromVp(0, vp).rval().setUndefined();
  return true;
}
struct Member { const char* name; const char* kind; const char* text; double number; };
struct Iface { const char* name; bool onPrototype; const Member* members; size_t count; };
const Member kMembers0[] = {
  {"rangeEnd", "GETSET", "", 0.0},
  {"rangeStart", "GETSET", "", 0.0},
};
const Member kMembers1[] = {
  {"duration", "GET", "", 0.0},
};
const Member kMembers2[] = {
  {"-moz-osx-font-smoothing", "GETSET", "", 0.0},
  {"-webkit-font-smoothing", "GETSET", "", 0.0},
  {"MozOsxFontSmoothing", "GETSET", "", 0.0},
  {"WebkitFontSmoothing", "GETSET", "", 0.0},
  {"animation-range", "GETSET", "", 0.0},
  {"animation-range-end", "GETSET", "", 0.0},
  {"animation-range-start", "GETSET", "", 0.0},
  {"animation-timeline", "GETSET", "", 0.0},
  {"animationRange", "GETSET", "", 0.0},
  {"animationRangeEnd", "GETSET", "", 0.0},
  {"animationRangeStart", "GETSET", "", 0.0},
  {"animationTimeline", "GETSET", "", 0.0},
  {"scroll-timeline", "GETSET", "", 0.0},
  {"scroll-timeline-axis", "GETSET", "", 0.0},
  {"scroll-timeline-name", "GETSET", "", 0.0},
  {"scrollTimeline", "GETSET", "", 0.0},
  {"scrollTimelineAxis", "GETSET", "", 0.0},
  {"scrollTimelineName", "GETSET", "", 0.0},
  {"timeline-scope", "GETSET", "", 0.0},
  {"timelineScope", "GETSET", "", 0.0},
  {"view-timeline", "GETSET", "", 0.0},
  {"view-timeline-axis", "GETSET", "", 0.0},
  {"view-timeline-inset", "GETSET", "", 0.0},
  {"view-timeline-name", "GETSET", "", 0.0},
  {"viewTimeline", "GETSET", "", 0.0},
  {"viewTimelineAxis", "GETSET", "", 0.0},
  {"viewTimelineInset", "GETSET", "", 0.0},
  {"viewTimelineName", "GETSET", "", 0.0},
  {"webkitFontSmoothing", "GETSET", "", 0.0},
};
const Member kMembers3[] = {
  {"initialize", "FN", "0", 1.0},
};
const Member kMembers4[] = {
  {"addElement", "FN", "0", 1.0},
  {"mozCursor", "GETSET", "", 0.0},
  {"mozSourceNode", "GET", "", 0.0},
  {"mozUserCancelled", "GET", "", 0.0},
};
const Member kMembers5[] = {
  {"webkitGetAsEntry", "FN", "0", 0.0},
};
const Member kMembers6[] = {
  {"customElementRegistry", "GET", "", 0.0},
  {"startViewTransition", "FN", "0", 0.0},
};
const Member kMembers7[] = {
  {"moveBefore", "FN", "0", 2.0},
};
const Member kMembers8[] = {
  {"ariaActiveDescendantElement", "GETSET", "", 0.0},
  {"ariaControlsElements", "GETSET", "", 0.0},
  {"ariaDescribedByElements", "GETSET", "", 0.0},
  {"ariaDetailsElements", "GETSET", "", 0.0},
  {"ariaErrorMessageElements", "GETSET", "", 0.0},
  {"ariaFlowToElements", "GETSET", "", 0.0},
  {"ariaLabelledByElements", "GETSET", "", 0.0},
  {"ariaNotify", "FN", "0", 1.0},
  {"ariaOwnsElements", "GETSET", "", 0.0},
  {"customElementRegistry", "GET", "", 0.0},
  {"hasPointerCapture", "FN", "0", 1.0},
  {"mozMatchesSelector", "FN", "0", 1.0},
  {"mozRequestFullScreen", "FN", "0", 0.0},
  {"releaseCapture", "FN", "0", 0.0},
  {"releasePointerCapture", "FN", "0", 1.0},
  {"requestFullscreen", "FN", "0", 0.0},
  {"requestPointerLock", "FN", "0", 0.0},
  {"setCapture", "FN", "0", 0.0},
  {"setPointerCapture", "FN", "0", 1.0},
  {"webkitMatchesSelector", "FN", "0", 1.0},
};
const Member kMembers9[] = {
  {"ascentOverride", "GETSET", "", 0.0},
  {"descentOverride", "GETSET", "", 0.0},
  {"lineGapOverride", "GETSET", "", 0.0},
  {"sizeAdjust", "GETSET", "", 0.0},
  {"variationSettings", "GETSET", "", 0.0},
  {"width", "GETSET", "", 0.0},
};
const Member kMembers10[] = {
  {"beginComputePass", "FN", "0", 0.0},
  {"clearBuffer", "FN", "0", 1.0},
  {"copyBufferToBuffer", "FN", "0", 2.0},
  {"copyBufferToTexture", "FN", "0", 3.0},
  {"copyTextureToBuffer", "FN", "0", 3.0},
  {"copyTextureToTexture", "FN", "0", 3.0},
  {"insertDebugMarker", "FN", "0", 1.0},
  {"popDebugGroup", "FN", "0", 0.0},
  {"pushDebugGroup", "FN", "0", 1.0},
  {"resolveQuerySet", "FN", "0", 5.0},
};
const Member kMembers11[] = {
  {"adapterInfo", "GET", "", 0.0},
  {"createBindGroup", "FN", "0", 1.0},
  {"createBindGroupLayout", "FN", "0", 1.0},
  {"createComputePipeline", "FN", "0", 1.0},
  {"createComputePipelineAsync", "FN", "0", 1.0},
  {"createPipelineLayout", "FN", "0", 1.0},
  {"createQuerySet", "FN", "0", 1.0},
  {"createRenderBundleEncoder", "FN", "0", 1.0},
  {"createRenderPipeline", "FN", "0", 1.0},
  {"createRenderPipelineAsync", "FN", "0", 1.0},
  {"createSampler", "FN", "0", 0.0},
  {"createShaderModule", "FN", "0", 1.0},
  {"importExternalTexture", "FN", "0", 1.0},
  {"label", "GETSET", "", 0.0},
  {"popErrorScope", "FN", "0", 0.0},
  {"pushErrorScope", "FN", "0", 1.0},
};
const Member kMembers12[] = {
  {"copyExternalImageToTexture", "FN", "0", 3.0},
  {"label", "GETSET", "", 0.0},
  {"writeBuffer", "FN", "0", 3.0},
  {"writeTexture", "FN", "0", 4.0},
};
const Member kMembers13[] = {
  {"beginOcclusionQuery", "FN", "0", 1.0},
  {"draw", "FN", "0", 1.0},
  {"drawIndexed", "FN", "0", 1.0},
  {"drawIndexedIndirect", "FN", "0", 2.0},
  {"drawIndirect", "FN", "0", 2.0},
  {"endOcclusionQuery", "FN", "0", 0.0},
  {"executeBundles", "FN", "0", 1.0},
  {"insertDebugMarker", "FN", "0", 1.0},
  {"popDebugGroup", "FN", "0", 0.0},
  {"pushDebugGroup", "FN", "0", 1.0},
  {"setBindGroup", "FN", "0", 2.0},
  {"setBlendConstant", "FN", "0", 1.0},
  {"setIndexBuffer", "FN", "0", 2.0},
  {"setPipeline", "FN", "0", 1.0},
  {"setScissorRect", "FN", "0", 4.0},
  {"setStencilReference", "FN", "0", 1.0},
  {"setVertexBuffer", "FN", "0", 2.0},
  {"setViewport", "FN", "0", 6.0},
};
const Member kMembers14[] = {
  {"relList", "GETSET", "", 0.0},
};
const Member kMembers15[] = {
  {"hreflang", "GETSET", "", 0.0},
  {"toString", "FN", "0", 0.0},
  {"type", "GETSET", "", 0.0},
};
const Member kMembers16[] = {
  {"command", "GETSET", "", 0.0},
  {"commandForElement", "GETSET", "", 0.0},
  {"popoverTargetAction", "GETSET", "", 0.0},
  {"popoverTargetElement", "GETSET", "", 0.0},
};
const Member kMembers17[] = {
  {"closedBy", "GETSET", "", 0.0},
  {"requestClose", "FN", "0", 0.0},
};
const Member kMembers18[] = {
  {"accessKeyLabel", "GET", "", 0.0},
  {"autocorrect", "GETSET", "", 0.0},
};
const Member kMembers19[] = {
  {"rel", "GETSET", "", 0.0},
  {"relList", "GETSET", "", 0.0},
};
const Member kMembers20[] = {
  {"colorSpace", "GETSET", "", 0.0},
  {"popoverTargetAction", "GETSET", "", 0.0},
  {"popoverTargetElement", "GETSET", "", 0.0},
  {"textLength", "GET", "", 0.0},
  {"webkitEntries", "GET", "", 0.0},
};
const Member kMembers21[] = {
  {"type", "GETSET", "", 0.0},
};
const Member kMembers22[] = {
  {"HAVE_CURRENT_DATA", "VAL", "2", 2.0},
  {"HAVE_ENOUGH_DATA", "VAL", "4", 4.0},
  {"HAVE_FUTURE_DATA", "VAL", "3", 3.0},
  {"HAVE_METADATA", "VAL", "1", 1.0},
  {"HAVE_NOTHING", "VAL", "0", 0.0},
  {"NETWORK_EMPTY", "VAL", "0", 0.0},
  {"NETWORK_IDLE", "VAL", "1", 1.0},
  {"NETWORK_LOADING", "VAL", "2", 2.0},
  {"NETWORK_NO_SOURCE", "VAL", "3", 3.0},
  {"buffered", "GET", "", 0.0},
  {"captureStream", "FN", "0", 0.0},
  {"crossOrigin", "GETSET", "", 0.0},
  {"defaultPlaybackRate", "GETSET", "", 0.0},
  {"duration", "GET", "", 0.0},
  {"error", "GET", "", 0.0},
  {"mozAudioCaptured", "GET", "", 0.0},
  {"mozCaptureStream", "FN", "0", 0.0},
  {"mozCaptureStreamUntilEnded", "FN", "0", 0.0},
  {"mozFragmentEnd", "GET", "", 0.0},
  {"mozGetMetadata", "FN", "0", 0.0},
  {"playbackRate", "GETSET", "", 0.0},
  {"played", "GET", "", 0.0},
  {"preservesPitch", "GETSET", "", 0.0},
  {"seekable", "GET", "", 0.0},
  {"seeking", "GET", "", 0.0},
  {"setSinkId", "FN", "0", 1.0},
  {"sinkId", "GET", "", 0.0},
  {"srcObject", "GETSET", "", 0.0},
};
const Member kMembers23[] = {
  {"align", "GETSET", "", 0.0},
  {"archive", "GETSET", "", 0.0},
  {"border", "GETSET", "", 0.0},
  {"checkValidity", "FN", "0", 0.0},
  {"code", "GETSET", "", 0.0},
  {"codeBase", "GETSET", "", 0.0},
  {"codeType", "GETSET", "", 0.0},
  {"contentDocument", "GET", "", 0.0},
  {"contentWindow", "GET", "", 0.0},
  {"data", "GETSET", "", 0.0},
  {"declare", "GETSET", "", 0.0},
  {"form", "GET", "", 0.0},
  {"getSVGDocument", "FN", "0", 0.0},
  {"height", "GETSET", "", 0.0},
  {"hspace", "GETSET", "", 0.0},
  {"name", "GETSET", "", 0.0},
  {"reportValidity", "FN", "0", 0.0},
  {"setCustomValidity", "FN", "0", 1.0},
  {"standby", "GETSET", "", 0.0},
  {"type", "GETSET", "", 0.0},
  {"useMap", "GETSET", "", 0.0},
  {"validationMessage", "GET", "", 0.0},
  {"validity", "GET", "", 0.0},
  {"vspace", "GETSET", "", 0.0},
  {"width", "GETSET", "", 0.0},
  {"willValidate", "GET", "", 0.0},
};
const Member kMembers24[] = {
  {"innerText", "GETSET", "", 0.0},
};
const Member kMembers25[] = {
  {"shadowRootClonable", "GETSET", "", 0.0},
  {"shadowRootCustomElementRegistry", "GETSET", "", 0.0},
  {"shadowRootDelegatesFocus", "GETSET", "", 0.0},
  {"shadowRootMode", "GETSET", "", 0.0},
  {"shadowRootSerializable", "GETSET", "", 0.0},
  {"shadowRootSlotAssignment", "GETSET", "", 0.0},
};
const Member kMembers26[] = {
  {"cancelVideoFrameCallback", "FN", "0", 1.0},
  {"disablePictureInPicture", "GETSET", "", 0.0},
  {"getVideoPlaybackQuality", "FN", "0", 0.0},
  {"height", "GETSET", "", 0.0},
  {"mozDecodedFrames", "GET", "", 0.0},
  {"mozFrameDelay", "GET", "", 0.0},
  {"mozHasAudio", "GET", "", 0.0},
  {"mozPaintedFrames", "GET", "", 0.0},
  {"mozParsedFrames", "GET", "", 0.0},
  {"mozPresentedFrames", "GET", "", 0.0},
  {"onenterpictureinpicture", "GETSET", "", 0.0},
  {"onleavepictureinpicture", "GETSET", "", 0.0},
  {"poster", "GETSET", "", 0.0},
  {"requestPictureInPicture", "FN", "0", 0.0},
  {"requestVideoFrameCallback", "FN", "0", 1.0},
  {"videoHeight", "GET", "", 0.0},
  {"videoWidth", "GET", "", 0.0},
  {"width", "GETSET", "", 0.0},
};
const Member kMembers27[] = {
  {"deleteObjectStore", "FN", "0", 1.0},
};
const Member kMembers28[] = {
  {"autofocus", "GETSET", "", 0.0},
  {"blur", "FN", "0", 0.0},
  {"focus", "FN", "0", 0.0},
  {"nonce", "GETSET", "", 0.0},
};
const Member kMembers29[] = {
  {"hasUAVisualTransition", "GET", "", 0.0},
  {"sourceElement", "GET", "", 0.0},
};
const Member kMembers30[] = {
  {"serial", "GET", "", 0.0},
};
const Member kMembers31[] = {
  {"navigate", "GET", "", 0.0},
};
const Member kMembers32[] = {
  {"persistentDeviceId", "GET", "", 0.0},
};
const Member kMembers33[] = {
  {"hasUAVisualTransition", "GET", "", 0.0},
};
const Member kMembers34[] = {
  {"address", "GET", "", 0.0},
  {"component", "GET", "", 0.0},
  {"foundation", "GET", "", 0.0},
  {"port", "GET", "", 0.0},
  {"priority", "GET", "", 0.0},
  {"protocol", "GET", "", 0.0},
  {"relatedAddress", "GET", "", 0.0},
  {"relatedPort", "GET", "", 0.0},
  {"tcpType", "GET", "", 0.0},
  {"type", "GET", "", 0.0},
};
const Member kMembers35[] = {
  {"addStream", "FN", "0", 1.0},
  {"canTrickleIceCandidates", "GET", "", 0.0},
  {"currentLocalDescription", "GET", "", 0.0},
  {"currentRemoteDescription", "GET", "", 0.0},
  {"getConfiguration", "FN", "0", 0.0},
  {"getIdentityAssertion", "FN", "0", 0.0},
  {"getLocalStreams", "FN", "0", 0.0},
  {"getRemoteStreams", "FN", "0", 0.0},
  {"getStats", "FN", "0", 0.0},
  {"idpLoginUrl", "GET", "", 0.0},
  {"onaddstream", "GETSET", "", 0.0},
  {"onaddtrack", "GETSET", "", 0.0},
  {"peerIdentity", "GET", "", 0.0},
  {"pendingLocalDescription", "GET", "", 0.0},
  {"pendingRemoteDescription", "GET", "", 0.0},
  {"removeTrack", "FN", "0", 1.0},
  {"restartIce", "FN", "0", 0.0},
  {"sctp", "GET", "", 0.0},
  {"setConfiguration", "FN", "0", 0.0},
  {"setIdentityProvider", "FN", "0", 1.0},
};
const Member kMembers36[] = {
  {"getContributingSources", "FN", "0", 0.0},
  {"getParameters", "FN", "0", 0.0},
  {"getStats", "FN", "0", 0.0},
  {"getSynchronizationSources", "FN", "0", 0.0},
  {"jitterBufferTarget", "GETSET", "", 0.0},
  {"transform", "GETSET", "", 0.0},
  {"transport", "GET", "", 0.0},
};
const Member kMembers37[] = {
  {"dtmf", "GET", "", 0.0},
  {"getParameters", "FN", "0", 0.0},
  {"getStats", "FN", "0", 0.0},
  {"replaceTrack", "FN", "0", 1.0},
  {"setParameters", "FN", "0", 1.0},
  {"setStreams", "FN", "0", 0.0},
  {"transform", "GETSET", "", 0.0},
  {"transport", "GET", "", 0.0},
};
const Member kMembers38[] = {
  {"pipeThrough", "FN", "0", 1.0},
  {"values", "FN", "0", 0.0},
};
const Member kMembers39[] = {
  {"from", "FN", "0", 1.0},
};
const Member kMembers40[] = {
  {"download", "GETSET", "", 0.0},
  {"hash", "GETSET", "", 0.0},
  {"host", "GETSET", "", 0.0},
  {"hostname", "GETSET", "", 0.0},
  {"href", "GET", "", 0.0},
  {"hreflang", "GETSET", "", 0.0},
  {"origin", "GET", "", 0.0},
  {"password", "GETSET", "", 0.0},
  {"pathname", "GETSET", "", 0.0},
  {"ping", "GETSET", "", 0.0},
  {"port", "GETSET", "", 0.0},
  {"protocol", "GETSET", "", 0.0},
  {"referrerPolicy", "GETSET", "", 0.0},
  {"rel", "GETSET", "", 0.0},
  {"relList", "GETSET", "", 0.0},
  {"search", "GETSET", "", 0.0},
  {"target", "GET", "", 0.0},
  {"type", "GETSET", "", 0.0},
  {"username", "GETSET", "", 0.0},
};
const Member kMembers41[] = {
  {"beginElement", "FN", "0", 0.0},
  {"beginElementAt", "FN", "0", 1.0},
  {"endElement", "FN", "0", 0.0},
  {"endElementAt", "FN", "0", 1.0},
  {"getCurrentTime", "FN", "0", 0.0},
  {"getSimpleDuration", "FN", "0", 0.0},
  {"getStartTime", "FN", "0", 0.0},
  {"onbegin", "GETSET", "", 0.0},
  {"onend", "GETSET", "", 0.0},
  {"onrepeat", "GETSET", "", 0.0},
  {"requiredExtensions", "GET", "", 0.0},
  {"systemLanguage", "GET", "", 0.0},
  {"targetElement", "GET", "", 0.0},
};
const Member kMembers42[] = {
  {"clipPathUnits", "GET", "", 0.0},
  {"transform", "GET", "", 0.0},
};
const Member kMembers43[] = {
  {"SVG_FECOMPONENTTRANSFER_TYPE_DISCRETE", "VAL", "3", 3.0},
  {"SVG_FECOMPONENTTRANSFER_TYPE_GAMMA", "VAL", "5", 5.0},
  {"SVG_FECOMPONENTTRANSFER_TYPE_IDENTITY", "VAL", "1", 1.0},
  {"SVG_FECOMPONENTTRANSFER_TYPE_LINEAR", "VAL", "4", 4.0},
  {"SVG_FECOMPONENTTRANSFER_TYPE_TABLE", "VAL", "2", 2.0},
  {"SVG_FECOMPONENTTRANSFER_TYPE_UNKNOWN", "VAL", "0", 0.0},
  {"amplitude", "GET", "", 0.0},
  {"exponent", "GET", "", 0.0},
  {"intercept", "GET", "", 0.0},
  {"offset", "GET", "", 0.0},
  {"slope", "GET", "", 0.0},
  {"tableValues", "GET", "", 0.0},
  {"type", "GET", "", 0.0},
};
const Member kMembers44[] = {
  {"SVG_FECOMPONENTTRANSFER_TYPE_DISCRETE", "VAL", "3", 3.0},
  {"SVG_FECOMPONENTTRANSFER_TYPE_GAMMA", "VAL", "5", 5.0},
  {"SVG_FECOMPONENTTRANSFER_TYPE_IDENTITY", "VAL", "1", 1.0},
  {"SVG_FECOMPONENTTRANSFER_TYPE_LINEAR", "VAL", "4", 4.0},
  {"SVG_FECOMPONENTTRANSFER_TYPE_TABLE", "VAL", "2", 2.0},
  {"SVG_FECOMPONENTTRANSFER_TYPE_UNKNOWN", "VAL", "0", 0.0},
};
const Member kMembers45[] = {
  {"autofocus", "GETSET", "", 0.0},
  {"blur", "FN", "0", 0.0},
  {"focus", "FN", "0", 0.0},
  {"nonce", "GETSET", "", 0.0},
  {"ownerSVGElement", "GET", "", 0.0},
  {"viewportElement", "GET", "", 0.0},
};
const Member kMembers46[] = {
  {"SVG_FEBLEND_MODE_COLOR", "VAL", "15", 15.0},
  {"SVG_FEBLEND_MODE_COLOR_BURN", "VAL", "8", 8.0},
  {"SVG_FEBLEND_MODE_COLOR_DODGE", "VAL", "7", 7.0},
  {"SVG_FEBLEND_MODE_DARKEN", "VAL", "4", 4.0},
  {"SVG_FEBLEND_MODE_DIFFERENCE", "VAL", "11", 11.0},
  {"SVG_FEBLEND_MODE_EXCLUSION", "VAL", "12", 12.0},
  {"SVG_FEBLEND_MODE_HARD_LIGHT", "VAL", "9", 9.0},
  {"SVG_FEBLEND_MODE_HUE", "VAL", "13", 13.0},
  {"SVG_FEBLEND_MODE_LIGHTEN", "VAL", "5", 5.0},
  {"SVG_FEBLEND_MODE_LUMINOSITY", "VAL", "16", 16.0},
  {"SVG_FEBLEND_MODE_MULTIPLY", "VAL", "2", 2.0},
  {"SVG_FEBLEND_MODE_NORMAL", "VAL", "1", 1.0},
  {"SVG_FEBLEND_MODE_OVERLAY", "VAL", "6", 6.0},
  {"SVG_FEBLEND_MODE_SATURATION", "VAL", "14", 14.0},
  {"SVG_FEBLEND_MODE_SCREEN", "VAL", "3", 3.0},
  {"SVG_FEBLEND_MODE_SOFT_LIGHT", "VAL", "10", 10.0},
  {"SVG_FEBLEND_MODE_UNKNOWN", "VAL", "0", 0.0},
  {"in1", "GET", "", 0.0},
  {"in2", "GET", "", 0.0},
  {"mode", "GET", "", 0.0},
  {"result", "GET", "", 0.0},
};
const Member kMembers47[] = {
  {"SVG_FEBLEND_MODE_COLOR", "VAL", "15", 15.0},
  {"SVG_FEBLEND_MODE_COLOR_BURN", "VAL", "8", 8.0},
  {"SVG_FEBLEND_MODE_COLOR_DODGE", "VAL", "7", 7.0},
  {"SVG_FEBLEND_MODE_DARKEN", "VAL", "4", 4.0},
  {"SVG_FEBLEND_MODE_DIFFERENCE", "VAL", "11", 11.0},
  {"SVG_FEBLEND_MODE_EXCLUSION", "VAL", "12", 12.0},
  {"SVG_FEBLEND_MODE_HARD_LIGHT", "VAL", "9", 9.0},
  {"SVG_FEBLEND_MODE_HUE", "VAL", "13", 13.0},
  {"SVG_FEBLEND_MODE_LIGHTEN", "VAL", "5", 5.0},
  {"SVG_FEBLEND_MODE_LUMINOSITY", "VAL", "16", 16.0},
  {"SVG_FEBLEND_MODE_MULTIPLY", "VAL", "2", 2.0},
  {"SVG_FEBLEND_MODE_NORMAL", "VAL", "1", 1.0},
  {"SVG_FEBLEND_MODE_OVERLAY", "VAL", "6", 6.0},
  {"SVG_FEBLEND_MODE_SATURATION", "VAL", "14", 14.0},
  {"SVG_FEBLEND_MODE_SCREEN", "VAL", "3", 3.0},
  {"SVG_FEBLEND_MODE_SOFT_LIGHT", "VAL", "10", 10.0},
  {"SVG_FEBLEND_MODE_UNKNOWN", "VAL", "0", 0.0},
};
const Member kMembers48[] = {
  {"SVG_FECOLORMATRIX_TYPE_HUEROTATE", "VAL", "3", 3.0},
  {"SVG_FECOLORMATRIX_TYPE_LUMINANCETOALPHA", "VAL", "4", 4.0},
  {"SVG_FECOLORMATRIX_TYPE_MATRIX", "VAL", "1", 1.0},
  {"SVG_FECOLORMATRIX_TYPE_SATURATE", "VAL", "2", 2.0},
  {"SVG_FECOLORMATRIX_TYPE_UNKNOWN", "VAL", "0", 0.0},
  {"in1", "GET", "", 0.0},
  {"result", "GET", "", 0.0},
  {"type", "GET", "", 0.0},
  {"values", "GET", "", 0.0},
};
const Member kMembers49[] = {
  {"SVG_FECOLORMATRIX_TYPE_HUEROTATE", "VAL", "3", 3.0},
  {"SVG_FECOLORMATRIX_TYPE_LUMINANCETOALPHA", "VAL", "4", 4.0},
  {"SVG_FECOLORMATRIX_TYPE_MATRIX", "VAL", "1", 1.0},
  {"SVG_FECOLORMATRIX_TYPE_SATURATE", "VAL", "2", 2.0},
  {"SVG_FECOLORMATRIX_TYPE_UNKNOWN", "VAL", "0", 0.0},
};
const Member kMembers50[] = {
  {"in1", "GET", "", 0.0},
  {"result", "GET", "", 0.0},
};
const Member kMembers51[] = {
  {"SVG_FECOMPOSITE_OPERATOR_ARITHMETIC", "VAL", "6", 6.0},
  {"SVG_FECOMPOSITE_OPERATOR_ATOP", "VAL", "4", 4.0},
  {"SVG_FECOMPOSITE_OPERATOR_IN", "VAL", "2", 2.0},
  {"SVG_FECOMPOSITE_OPERATOR_LIGHTER", "VAL", "7", 7.0},
  {"SVG_FECOMPOSITE_OPERATOR_OUT", "VAL", "3", 3.0},
  {"SVG_FECOMPOSITE_OPERATOR_OVER", "VAL", "1", 1.0},
  {"SVG_FECOMPOSITE_OPERATOR_UNKNOWN", "VAL", "0", 0.0},
  {"SVG_FECOMPOSITE_OPERATOR_XOR", "VAL", "5", 5.0},
  {"in1", "GET", "", 0.0},
  {"in2", "GET", "", 0.0},
  {"k1", "GET", "", 0.0},
  {"k2", "GET", "", 0.0},
  {"k3", "GET", "", 0.0},
  {"k4", "GET", "", 0.0},
  {"operator", "GET", "", 0.0},
  {"result", "GET", "", 0.0},
};
const Member kMembers52[] = {
  {"SVG_FECOMPOSITE_OPERATOR_ARITHMETIC", "VAL", "6", 6.0},
  {"SVG_FECOMPOSITE_OPERATOR_ATOP", "VAL", "4", 4.0},
  {"SVG_FECOMPOSITE_OPERATOR_IN", "VAL", "2", 2.0},
  {"SVG_FECOMPOSITE_OPERATOR_LIGHTER", "VAL", "7", 7.0},
  {"SVG_FECOMPOSITE_OPERATOR_OUT", "VAL", "3", 3.0},
  {"SVG_FECOMPOSITE_OPERATOR_OVER", "VAL", "1", 1.0},
  {"SVG_FECOMPOSITE_OPERATOR_UNKNOWN", "VAL", "0", 0.0},
  {"SVG_FECOMPOSITE_OPERATOR_XOR", "VAL", "5", 5.0},
};
const Member kMembers53[] = {
  {"SVG_EDGEMODE_DUPLICATE", "VAL", "1", 1.0},
  {"SVG_EDGEMODE_NONE", "VAL", "3", 3.0},
  {"SVG_EDGEMODE_UNKNOWN", "VAL", "0", 0.0},
  {"SVG_EDGEMODE_WRAP", "VAL", "2", 2.0},
  {"bias", "GET", "", 0.0},
  {"divisor", "GET", "", 0.0},
  {"edgeMode", "GET", "", 0.0},
  {"in1", "GET", "", 0.0},
  {"kernelMatrix", "GET", "", 0.0},
  {"kernelUnitLengthX", "GET", "", 0.0},
  {"kernelUnitLengthY", "GET", "", 0.0},
  {"orderX", "GET", "", 0.0},
  {"orderY", "GET", "", 0.0},
  {"preserveAlpha", "GET", "", 0.0},
  {"result", "GET", "", 0.0},
  {"targetX", "GET", "", 0.0},
  {"targetY", "GET", "", 0.0},
};
const Member kMembers54[] = {
  {"SVG_EDGEMODE_DUPLICATE", "VAL", "1", 1.0},
  {"SVG_EDGEMODE_NONE", "VAL", "3", 3.0},
  {"SVG_EDGEMODE_UNKNOWN", "VAL", "0", 0.0},
  {"SVG_EDGEMODE_WRAP", "VAL", "2", 2.0},
};
const Member kMembers55[] = {
  {"diffuseConstant", "GET", "", 0.0},
  {"in1", "GET", "", 0.0},
  {"kernelUnitLengthX", "GET", "", 0.0},
  {"kernelUnitLengthY", "GET", "", 0.0},
  {"result", "GET", "", 0.0},
  {"surfaceScale", "GET", "", 0.0},
};
const Member kMembers56[] = {
  {"SVG_CHANNEL_A", "VAL", "4", 4.0},
  {"SVG_CHANNEL_B", "VAL", "3", 3.0},
  {"SVG_CHANNEL_G", "VAL", "2", 2.0},
  {"SVG_CHANNEL_R", "VAL", "1", 1.0},
  {"SVG_CHANNEL_UNKNOWN", "VAL", "0", 0.0},
  {"in1", "GET", "", 0.0},
  {"in2", "GET", "", 0.0},
  {"result", "GET", "", 0.0},
  {"scale", "GET", "", 0.0},
  {"xChannelSelector", "GET", "", 0.0},
  {"yChannelSelector", "GET", "", 0.0},
};
const Member kMembers57[] = {
  {"SVG_CHANNEL_A", "VAL", "4", 4.0},
  {"SVG_CHANNEL_B", "VAL", "3", 3.0},
  {"SVG_CHANNEL_G", "VAL", "2", 2.0},
  {"SVG_CHANNEL_R", "VAL", "1", 1.0},
  {"SVG_CHANNEL_UNKNOWN", "VAL", "0", 0.0},
};
const Member kMembers58[] = {
  {"azimuth", "GET", "", 0.0},
  {"elevation", "GET", "", 0.0},
};
const Member kMembers59[] = {
  {"dx", "GET", "", 0.0},
  {"dy", "GET", "", 0.0},
  {"in1", "GET", "", 0.0},
  {"result", "GET", "", 0.0},
  {"setStdDeviation", "FN", "0", 2.0},
  {"stdDeviationX", "GET", "", 0.0},
  {"stdDeviationY", "GET", "", 0.0},
};
const Member kMembers60[] = {
  {"result", "GET", "", 0.0},
};
const Member kMembers61[] = {
  {"in1", "GET", "", 0.0},
  {"result", "GET", "", 0.0},
  {"setStdDeviation", "FN", "0", 2.0},
  {"stdDeviationX", "GET", "", 0.0},
  {"stdDeviationY", "GET", "", 0.0},
};
const Member kMembers62[] = {
  {"crossOrigin", "GETSET", "", 0.0},
  {"fetchPriority", "GETSET", "", 0.0},
  {"href", "GET", "", 0.0},
  {"preserveAspectRatio", "GET", "", 0.0},
  {"result", "GET", "", 0.0},
};
const Member kMembers63[] = {
  {"result", "GET", "", 0.0},
};
const Member kMembers64[] = {
  {"in1", "GET", "", 0.0},
};
const Member kMembers65[] = {
  {"SVG_MORPHOLOGY_OPERATOR_DILATE", "VAL", "2", 2.0},
  {"SVG_MORPHOLOGY_OPERATOR_ERODE", "VAL", "1", 1.0},
  {"SVG_MORPHOLOGY_OPERATOR_UNKNOWN", "VAL", "0", 0.0},
  {"in1", "GET", "", 0.0},
  {"operator", "GET", "", 0.0},
  {"radiusX", "GET", "", 0.0},
  {"radiusY", "GET", "", 0.0},
  {"result", "GET", "", 0.0},
};
const Member kMembers66[] = {
  {"SVG_MORPHOLOGY_OPERATOR_DILATE", "VAL", "2", 2.0},
  {"SVG_MORPHOLOGY_OPERATOR_ERODE", "VAL", "1", 1.0},
  {"SVG_MORPHOLOGY_OPERATOR_UNKNOWN", "VAL", "0", 0.0},
};
const Member kMembers67[] = {
  {"dx", "GET", "", 0.0},
  {"dy", "GET", "", 0.0},
  {"in1", "GET", "", 0.0},
  {"result", "GET", "", 0.0},
};
const Member kMembers68[] = {
  {"x", "GET", "", 0.0},
  {"y", "GET", "", 0.0},
  {"z", "GET", "", 0.0},
};
const Member kMembers69[] = {
  {"in1", "GET", "", 0.0},
  {"kernelUnitLengthX", "GET", "", 0.0},
  {"kernelUnitLengthY", "GET", "", 0.0},
  {"result", "GET", "", 0.0},
  {"specularConstant", "GET", "", 0.0},
  {"specularExponent", "GET", "", 0.0},
  {"surfaceScale", "GET", "", 0.0},
};
const Member kMembers70[] = {
  {"limitingConeAngle", "GET", "", 0.0},
  {"pointsAtX", "GET", "", 0.0},
  {"pointsAtY", "GET", "", 0.0},
  {"pointsAtZ", "GET", "", 0.0},
  {"specularExponent", "GET", "", 0.0},
  {"x", "GET", "", 0.0},
  {"y", "GET", "", 0.0},
  {"z", "GET", "", 0.0},
};
const Member kMembers71[] = {
  {"in1", "GET", "", 0.0},
  {"result", "GET", "", 0.0},
};
const Member kMembers72[] = {
  {"SVG_STITCHTYPE_NOSTITCH", "VAL", "2", 2.0},
  {"SVG_STITCHTYPE_STITCH", "VAL", "1", 1.0},
  {"SVG_STITCHTYPE_UNKNOWN", "VAL", "0", 0.0},
  {"SVG_TURBULENCE_TYPE_FRACTALNOISE", "VAL", "1", 1.0},
  {"SVG_TURBULENCE_TYPE_TURBULENCE", "VAL", "2", 2.0},
  {"SVG_TURBULENCE_TYPE_UNKNOWN", "VAL", "0", 0.0},
  {"baseFrequencyX", "GET", "", 0.0},
  {"baseFrequencyY", "GET", "", 0.0},
  {"numOctaves", "GET", "", 0.0},
  {"result", "GET", "", 0.0},
  {"seed", "GET", "", 0.0},
  {"stitchTiles", "GET", "", 0.0},
  {"type", "GET", "", 0.0},
};
const Member kMembers73[] = {
  {"SVG_STITCHTYPE_NOSTITCH", "VAL", "2", 2.0},
  {"SVG_STITCHTYPE_STITCH", "VAL", "1", 1.0},
  {"SVG_STITCHTYPE_UNKNOWN", "VAL", "0", 0.0},
  {"SVG_TURBULENCE_TYPE_FRACTALNOISE", "VAL", "1", 1.0},
  {"SVG_TURBULENCE_TYPE_TURBULENCE", "VAL", "2", 2.0},
  {"SVG_TURBULENCE_TYPE_UNKNOWN", "VAL", "0", 0.0},
};
const Member kMembers74[] = {
  {"filterUnits", "GET", "", 0.0},
  {"href", "GET", "", 0.0},
  {"primitiveUnits", "GET", "", 0.0},
};
const Member kMembers75[] = {
  {"getPointAtLength", "FN", "0", 1.0},
  {"getTotalLength", "FN", "0", 0.0},
  {"isPointInFill", "FN", "0", 0.0},
  {"isPointInStroke", "FN", "0", 0.0},
  {"pathLength", "GET", "", 0.0},
};
const Member kMembers76[] = {
  {"SVG_SPREADMETHOD_PAD", "VAL", "1", 1.0},
  {"SVG_SPREADMETHOD_REFLECT", "VAL", "2", 2.0},
  {"SVG_SPREADMETHOD_REPEAT", "VAL", "3", 3.0},
  {"SVG_SPREADMETHOD_UNKNOWN", "VAL", "0", 0.0},
  {"gradientTransform", "GET", "", 0.0},
  {"gradientUnits", "GET", "", 0.0},
  {"href", "GET", "", 0.0},
  {"spreadMethod", "GET", "", 0.0},
};
const Member kMembers77[] = {
  {"SVG_SPREADMETHOD_PAD", "VAL", "1", 1.0},
  {"SVG_SPREADMETHOD_REFLECT", "VAL", "2", 2.0},
  {"SVG_SPREADMETHOD_REPEAT", "VAL", "3", 3.0},
  {"SVG_SPREADMETHOD_UNKNOWN", "VAL", "0", 0.0},
};
const Member kMembers78[] = {
  {"getCTM", "FN", "0", 0.0},
  {"getScreenCTM", "FN", "0", 0.0},
  {"requiredExtensions", "GET", "", 0.0},
  {"systemLanguage", "GET", "", 0.0},
  {"transform", "GET", "", 0.0},
};
const Member kMembers79[] = {
  {"crossOrigin", "GETSET", "", 0.0},
  {"decode", "FN", "0", 0.0},
  {"decoding", "GETSET", "", 0.0},
  {"fetchPriority", "GETSET", "", 0.0},
  {"href", "GET", "", 0.0},
  {"preserveAspectRatio", "GET", "", 0.0},
};
const Member kMembers80[] = {
  {"href", "GET", "", 0.0},
};
const Member kMembers81[] = {
  {"SVG_MARKERUNITS_STROKEWIDTH", "VAL", "2", 2.0},
  {"SVG_MARKERUNITS_UNKNOWN", "VAL", "0", 0.0},
  {"SVG_MARKERUNITS_USERSPACEONUSE", "VAL", "1", 1.0},
  {"SVG_MARKER_ORIENT_ANGLE", "VAL", "2", 2.0},
  {"SVG_MARKER_ORIENT_AUTO", "VAL", "1", 1.0},
  {"SVG_MARKER_ORIENT_AUTO_START_REVERSE", "VAL", "3", 3.0},
  {"SVG_MARKER_ORIENT_UNKNOWN", "VAL", "0", 0.0},
  {"markerUnits", "GET", "", 0.0},
  {"orientAngle", "GET", "", 0.0},
  {"orientType", "GET", "", 0.0},
  {"preserveAspectRatio", "GET", "", 0.0},
  {"setOrientToAngle", "FN", "0", 1.0},
  {"setOrientToAuto", "FN", "0", 0.0},
};
const Member kMembers82[] = {
  {"SVG_MARKERUNITS_STROKEWIDTH", "VAL", "2", 2.0},
  {"SVG_MARKERUNITS_UNKNOWN", "VAL", "0", 0.0},
  {"SVG_MARKERUNITS_USERSPACEONUSE", "VAL", "1", 1.0},
  {"SVG_MARKER_ORIENT_ANGLE", "VAL", "2", 2.0},
  {"SVG_MARKER_ORIENT_AUTO", "VAL", "1", 1.0},
  {"SVG_MARKER_ORIENT_AUTO_START_REVERSE", "VAL", "3", 3.0},
  {"SVG_MARKER_ORIENT_UNKNOWN", "VAL", "0", 0.0},
};
const Member kMembers83[] = {
  {"SVG_MASKTYPE_ALPHA", "VAL", "1", 1.0},
  {"SVG_MASKTYPE_LUMINANCE", "VAL", "0", 0.0},
  {"maskContentUnits", "GET", "", 0.0},
  {"maskUnits", "GET", "", 0.0},
};
const Member kMembers84[] = {
  {"SVG_MASKTYPE_ALPHA", "VAL", "1", 1.0},
  {"SVG_MASKTYPE_LUMINANCE", "VAL", "0", 0.0},
};
const Member kMembers85[] = {
  {"getPathData", "FN", "0", 0.0},
  {"getPathSegmentAtLength", "FN", "0", 1.0},
  {"setPathData", "FN", "0", 1.0},
};
const Member kMembers86[] = {
  {"href", "GET", "", 0.0},
  {"patternContentUnits", "GET", "", 0.0},
  {"patternTransform", "GET", "", 0.0},
  {"patternUnits", "GET", "", 0.0},
  {"preserveAspectRatio", "GET", "", 0.0},
};
const Member kMembers87[] = {
  {"SVG_ZOOMANDPAN_DISABLE", "VAL", "1", 1.0},
  {"SVG_ZOOMANDPAN_MAGNIFY", "VAL", "2", 2.0},
  {"SVG_ZOOMANDPAN_UNKNOWN", "VAL", "0", 0.0},
  {"animationsPaused", "FN", "0", 0.0},
  {"createSVGRect", "FN", "0", 0.0},
  {"createSVGTransformFromMatrix", "FN", "0", 0.0},
  {"currentScale", "GETSET", "", 0.0},
  {"currentTranslate", "GET", "", 0.0},
  {"deselectAll", "FN", "0", 0.0},
  {"forceRedraw", "FN", "0", 0.0},
  {"getCurrentTime", "FN", "0", 0.0},
  {"getElementById", "FN", "0", 1.0},
  {"pauseAnimations", "FN", "0", 0.0},
  {"preserveAspectRatio", "GET", "", 0.0},
  {"setCurrentTime", "FN", "0", 1.0},
  {"suspendRedraw", "FN", "0", 1.0},
  {"unpauseAnimations", "FN", "0", 0.0},
  {"unsuspendRedraw", "FN", "0", 1.0},
  {"unsuspendRedrawAll", "FN", "0", 0.0},
  {"zoomAndPan", "GETSET", "", 0.0},
};
const Member kMembers88[] = {
  {"async", "GETSET", "", 0.0},
  {"crossOrigin", "GETSET", "", 0.0},
  {"defer", "GETSET", "", 0.0},
  {"fetchPriority", "GETSET", "", 0.0},
  {"href", "GET", "", 0.0},
  {"type", "GETSET", "", 0.0},
};
const Member kMembers89[] = {
  {"offset", "GET", "", 0.0},
};
const Member kMembers90[] = {
  {"disabled", "GETSET", "", 0.0},
  {"media", "GETSET", "", 0.0},
  {"sheet", "GET", "", 0.0},
  {"title", "GETSET", "", 0.0},
  {"type", "GETSET", "", 0.0},
};
const Member kMembers91[] = {
  {"preserveAspectRatio", "GET", "", 0.0},
  {"requiredExtensions", "GET", "", 0.0},
  {"systemLanguage", "GET", "", 0.0},
};
const Member kMembers92[] = {
  {"LENGTHADJUST_SPACING", "VAL", "1", 1.0},
  {"LENGTHADJUST_SPACINGANDGLYPHS", "VAL", "2", 2.0},
  {"LENGTHADJUST_UNKNOWN", "VAL", "0", 0.0},
  {"getCharNumAtPosition", "FN", "0", 0.0},
  {"getEndPositionOfChar", "FN", "0", 1.0},
  {"getRotationOfChar", "FN", "0", 1.0},
  {"getStartPositionOfChar", "FN", "0", 1.0},
  {"lengthAdjust", "GET", "", 0.0},
  {"selectSubString", "FN", "0", 2.0},
};
const Member kMembers93[] = {
  {"LENGTHADJUST_SPACING", "VAL", "1", 1.0},
  {"LENGTHADJUST_SPACINGANDGLYPHS", "VAL", "2", 2.0},
  {"LENGTHADJUST_UNKNOWN", "VAL", "0", 0.0},
};
const Member kMembers94[] = {
  {"TEXTPATH_METHODTYPE_ALIGN", "VAL", "1", 1.0},
  {"TEXTPATH_METHODTYPE_STRETCH", "VAL", "2", 2.0},
  {"TEXTPATH_METHODTYPE_UNKNOWN", "VAL", "0", 0.0},
  {"TEXTPATH_SIDETYPE_LEFT", "VAL", "1", 1.0},
  {"TEXTPATH_SIDETYPE_RIGHT", "VAL", "2", 2.0},
  {"TEXTPATH_SIDETYPE_UNKNOWN", "VAL", "0", 0.0},
  {"TEXTPATH_SPACINGTYPE_AUTO", "VAL", "1", 1.0},
  {"TEXTPATH_SPACINGTYPE_EXACT", "VAL", "2", 2.0},
  {"TEXTPATH_SPACINGTYPE_UNKNOWN", "VAL", "0", 0.0},
  {"href", "GET", "", 0.0},
  {"method", "GET", "", 0.0},
  {"side", "GET", "", 0.0},
  {"spacing", "GET", "", 0.0},
};
const Member kMembers95[] = {
  {"TEXTPATH_METHODTYPE_ALIGN", "VAL", "1", 1.0},
  {"TEXTPATH_METHODTYPE_STRETCH", "VAL", "2", 2.0},
  {"TEXTPATH_METHODTYPE_UNKNOWN", "VAL", "0", 0.0},
  {"TEXTPATH_SIDETYPE_LEFT", "VAL", "1", 1.0},
  {"TEXTPATH_SIDETYPE_RIGHT", "VAL", "2", 2.0},
  {"TEXTPATH_SIDETYPE_UNKNOWN", "VAL", "0", 0.0},
  {"TEXTPATH_SPACINGTYPE_AUTO", "VAL", "1", 1.0},
  {"TEXTPATH_SPACINGTYPE_EXACT", "VAL", "2", 2.0},
  {"TEXTPATH_SPACINGTYPE_UNKNOWN", "VAL", "0", 0.0},
};
const Member kMembers96[] = {
  {"dx", "GET", "", 0.0},
  {"dy", "GET", "", 0.0},
  {"rotate", "GET", "", 0.0},
  {"x", "GET", "", 0.0},
  {"y", "GET", "", 0.0},
};
const Member kMembers97[] = {
  {"href", "GET", "", 0.0},
};
const Member kMembers98[] = {
  {"SVG_ZOOMANDPAN_DISABLE", "VAL", "1", 1.0},
  {"SVG_ZOOMANDPAN_MAGNIFY", "VAL", "2", 2.0},
  {"SVG_ZOOMANDPAN_UNKNOWN", "VAL", "0", 0.0},
  {"preserveAspectRatio", "GET", "", 0.0},
  {"zoomAndPan", "GETSET", "", 0.0},
};
const Member kMembers99[] = {
  {"SVG_ZOOMANDPAN_DISABLE", "VAL", "1", 1.0},
  {"SVG_ZOOMANDPAN_MAGNIFY", "VAL", "2", 2.0},
  {"SVG_ZOOMANDPAN_UNKNOWN", "VAL", "0", 0.0},
};
const Member kMembers100[] = {
  {"caretBidiLevel", "GETSET", "", 0.0},
  {"direction", "GET", "", 0.0},
  {"getComposedRanges", "FN", "0", 0.0},
};
const Member kMembers101[] = {
  {"customElementRegistry", "GET", "", 0.0},
  {"mozFullScreenElement", "GET", "", 0.0},
};
const Member kMembers102[] = {
  {"source", "GET", "", 0.0},
};
const Member kMembers103[] = {
  {"SCROLL_PAGE_DOWN", "VAL", "32768", 32768.0},
  {"SCROLL_PAGE_UP", "VAL", "-32768", -32768.0},
  {"rangeOffset", "GET", "", 0.0},
  {"rangeParent", "GET", "", 0.0},
};
const Member kMembers104[] = {
  {"SCROLL_PAGE_DOWN", "VAL", "32768", 32768.0},
  {"SCROLL_PAGE_UP", "VAL", "-32768", -32768.0},
};
const Member kMembers105[] = {
  {"TIMEOUT_IGNORED", "VAL", "-1", -1.0},
};
const Member kMembers106[] = {
  {"binaryType", "GETSET", "", 0.0},
  {"bufferedAmount", "GET", "", 0.0},
  {"extensions", "GET", "", 0.0},
  {"onclose", "GETSET", "", 0.0},
  {"onmessage", "GETSET", "", 0.0},
  {"onopen", "GETSET", "", 0.0},
  {"protocol", "GET", "", 0.0},
  {"readyState", "GET", "", 0.0},
  {"send", "FN", "0", 1.0},
  {"url", "GET", "", 0.0},
};
const Iface kIfaces[] = {
  {"Animation", true, kMembers0, 2},
  {"AnimationTimeline", true, kMembers1, 1},
  {"CSSStyleProperties", true, kMembers2, 29},
  {"CustomElementRegistry", true, kMembers3, 1},
  {"DataTransfer", true, kMembers4, 4},
  {"DataTransferItem", true, kMembers5, 1},
  {"Document", true, kMembers6, 2},
  {"DocumentFragment", true, kMembers7, 1},
  {"Element", true, kMembers8, 20},
  {"FontFace", true, kMembers9, 6},
  {"GPUCommandEncoder", true, kMembers10, 10},
  {"GPUDevice", true, kMembers11, 16},
  {"GPUQueue", true, kMembers12, 4},
  {"GPURenderPassEncoder", true, kMembers13, 18},
  {"HTMLAnchorElement", true, kMembers14, 1},
  {"HTMLAreaElement", true, kMembers15, 3},
  {"HTMLButtonElement", true, kMembers16, 4},
  {"HTMLDialogElement", true, kMembers17, 2},
  {"HTMLElement", true, kMembers18, 2},
  {"HTMLFormElement", true, kMembers19, 2},
  {"HTMLInputElement", true, kMembers20, 5},
  {"HTMLLIElement", true, kMembers21, 1},
  {"HTMLMediaElement", true, kMembers22, 28},
  {"HTMLObjectElement", true, kMembers23, 26},
  {"HTMLScriptElement", true, kMembers24, 1},
  {"HTMLTemplateElement", true, kMembers25, 6},
  {"HTMLVideoElement", true, kMembers26, 18},
  {"IDBDatabase", true, kMembers27, 1},
  {"MathMLElement", true, kMembers28, 4},
  {"NavigateEvent", true, kMembers29, 2},
  {"Navigator", true, kMembers30, 1},
  {"Notification", true, kMembers31, 1},
  {"PointerEvent", true, kMembers32, 1},
  {"PopStateEvent", true, kMembers33, 1},
  {"RTCIceCandidate", true, kMembers34, 10},
  {"RTCPeerConnection", true, kMembers35, 20},
  {"RTCRtpReceiver", true, kMembers36, 7},
  {"RTCRtpSender", true, kMembers37, 8},
  {"ReadableStream", true, kMembers38, 2},
  {"ReadableStream", false, kMembers39, 1},
  {"SVGAElement", true, kMembers40, 19},
  {"SVGAnimationElement", true, kMembers41, 13},
  {"SVGClipPathElement", true, kMembers42, 2},
  {"SVGComponentTransferFunctionElement", true, kMembers43, 13},
  {"SVGComponentTransferFunctionElement", false, kMembers44, 6},
  {"SVGElement", true, kMembers45, 6},
  {"SVGFEBlendElement", true, kMembers46, 21},
  {"SVGFEBlendElement", false, kMembers47, 17},
  {"SVGFEColorMatrixElement", true, kMembers48, 9},
  {"SVGFEColorMatrixElement", false, kMembers49, 5},
  {"SVGFEComponentTransferElement", true, kMembers50, 2},
  {"SVGFECompositeElement", true, kMembers51, 16},
  {"SVGFECompositeElement", false, kMembers52, 8},
  {"SVGFEConvolveMatrixElement", true, kMembers53, 17},
  {"SVGFEConvolveMatrixElement", false, kMembers54, 4},
  {"SVGFEDiffuseLightingElement", true, kMembers55, 6},
  {"SVGFEDisplacementMapElement", true, kMembers56, 11},
  {"SVGFEDisplacementMapElement", false, kMembers57, 5},
  {"SVGFEDistantLightElement", true, kMembers58, 2},
  {"SVGFEDropShadowElement", true, kMembers59, 7},
  {"SVGFEFloodElement", true, kMembers60, 1},
  {"SVGFEGaussianBlurElement", true, kMembers61, 5},
  {"SVGFEImageElement", true, kMembers62, 5},
  {"SVGFEMergeElement", true, kMembers63, 1},
  {"SVGFEMergeNodeElement", true, kMembers64, 1},
  {"SVGFEMorphologyElement", true, kMembers65, 8},
  {"SVGFEMorphologyElement", false, kMembers66, 3},
  {"SVGFEOffsetElement", true, kMembers67, 4},
  {"SVGFEPointLightElement", true, kMembers68, 3},
  {"SVGFESpecularLightingElement", true, kMembers69, 7},
  {"SVGFESpotLightElement", true, kMembers70, 8},
  {"SVGFETileElement", true, kMembers71, 2},
  {"SVGFETurbulenceElement", true, kMembers72, 13},
  {"SVGFETurbulenceElement", false, kMembers73, 6},
  {"SVGFilterElement", true, kMembers74, 3},
  {"SVGGeometryElement", true, kMembers75, 5},
  {"SVGGradientElement", true, kMembers76, 8},
  {"SVGGradientElement", false, kMembers77, 4},
  {"SVGGraphicsElement", true, kMembers78, 5},
  {"SVGImageElement", true, kMembers79, 6},
  {"SVGMPathElement", true, kMembers80, 1},
  {"SVGMarkerElement", true, kMembers81, 13},
  {"SVGMarkerElement", false, kMembers82, 7},
  {"SVGMaskElement", true, kMembers83, 4},
  {"SVGMaskElement", false, kMembers84, 2},
  {"SVGPathElement", true, kMembers85, 3},
  {"SVGPatternElement", true, kMembers86, 5},
  {"SVGSVGElement", true, kMembers87, 20},
  {"SVGScriptElement", true, kMembers88, 6},
  {"SVGStopElement", true, kMembers89, 1},
  {"SVGStyleElement", true, kMembers90, 5},
  {"SVGSymbolElement", true, kMembers91, 3},
  {"SVGTextContentElement", true, kMembers92, 9},
  {"SVGTextContentElement", false, kMembers93, 3},
  {"SVGTextPathElement", true, kMembers94, 13},
  {"SVGTextPathElement", false, kMembers95, 9},
  {"SVGTextPositioningElement", true, kMembers96, 5},
  {"SVGUseElement", true, kMembers97, 1},
  {"SVGViewElement", true, kMembers98, 5},
  {"SVGViewElement", false, kMembers99, 3},
  {"Selection", true, kMembers100, 3},
  {"ShadowRoot", true, kMembers101, 2},
  {"ToggleEvent", true, kMembers102, 1},
  {"UIEvent", true, kMembers103, 4},
  {"UIEvent", false, kMembers104, 2},
  {"WebGL2RenderingContext", true, kMembers105, 1},
  {"WebSocket", true, kMembers106, 10},
};
}  // namespace

bool InstallSurfaceParity(JSContext* cx, JS::HandleObject page) {
  for (const auto& iface : kIfaces) {
    JS::RootedValue holder(cx);
    if (!JS_GetProperty(cx, page, iface.name, &holder) || !holder.isObject()) continue;
    JS::RootedObject target(cx, &holder.toObject());
    if (iface.onPrototype) {
      JS::RootedValue proto(cx);
      if (!JS_GetProperty(cx, target, "prototype", &proto) || !proto.isObject()) continue;
      target = &proto.toObject();
    }
    for (size_t i = 0; i < iface.count; ++i) {
      const Member& member = iface.members[i];
      // Own keys only: an inherited name (toString on Object.prototype) must
      // still get its own FF159 member here.
      bool present = false;
      if (!JS_HasOwnProperty(cx, target, member.name, &present)) return false;
      if (present) continue;
      if (member.kind[0] == 'F') {  // FN
        if (!JS_DefineFunction(cx, target, member.name, SurfaceUndefined,
                               static_cast<unsigned>(member.number), 0)) return false;
      } else if (member.kind[0] == 'V') {  // VAL constant
        JS::RootedValue value(cx, JS::DoubleValue(member.number));
        if (!JS_DefineProperty(cx, target, member.name, value, 0)) return false;
      } else if (member.kind[3] == 'S' && member.kind[4] == 'E' && member.kind[5] == 'T') {  // GETSET
        const JSPropertySpec spec[] = {
            JS_PSGS(member.name, SurfaceUndefined, SurfaceIgnoreSet, 0), JS_PS_END};
        if (!JS_DefineProperties(cx, target, spec)) return false;
      } else {  // GET
        const JSPropertySpec spec[] = {
            JS_PSG(member.name, SurfaceUndefined, 0), JS_PS_END};
        if (!JS_DefineProperties(cx, target, spec)) return false;
      }
    }
  }
  return true;
}
}  // namespace ruyijs

