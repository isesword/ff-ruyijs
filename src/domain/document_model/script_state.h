#pragma once
#include <cstdint>
#include <string_view>
namespace ruyijs::domain::document_model {
struct ScriptState {
  bool started = false;
  bool parser_inserted = false;
  bool force_async = true;
  // Document position (1-based line, UTF-16 column) of the script text the
  // parser created, as Firefox's nsScriptElement line/column numbers; 0 when
  // the text did not come from the document's markup.
  uint32_t line = 0, column = 0;
};
// Cloudflare Rocket Loader rewrites script types to "<hex digest>-<essence>"
// (e.g. "41f22074...-text/javascript"); the digest length is not fixed, so a
// leading run of hex digits followed by '-' is stripped before matching. The
// stripped form is only accepted when the remainder matches a known essence
// (the callers), which keeps arbitrary hyphenated types from passing.
template<class Char> std::basic_string_view<Char> rocket_loader_essence(std::basic_string_view<Char> value) {
  auto hex=[](Char c){return (c>='0'&&c<='9')||(c>='a'&&c<='f')||(c>='A'&&c<='F');};
  size_t i=0;
  while(i<value.size()&&hex(value[i]))++i;
  if(i>=4&&i<value.size()&&value[i]==(Char)'-')return value.substr(i+1);
  return value;
}
// The "module" essence after Rocket Loader stripping: true when the script is
// an ES module (the module owner's pipeline, distinct from classic Execute).
template<class Char> bool script_module(std::basic_string_view<Char> value) {
  value = rocket_loader_essence(value);
  auto space=[](Char c){return c==9||c==10||c==12||c==13||c==32;};
  while(!value.empty()&&space(value.front()))value.remove_prefix(1);
  while(!value.empty()&&space(value.back()))value.remove_suffix(1);
  if(value.size()!=6)return false;
  constexpr Char expected[]={'m','o','d','u','l','e'};
  for(size_t i=0;i<6;++i){
    auto c=value[i];if(c>='A'&&c<='Z')c+=32;
    if(c!=expected[i])return false;
  }
  return true;
}

// HTML JavaScript MIME type essence matching, shared by preparation and loading.
template<class Char> bool script_classic(std::basic_string_view<Char> value) {
  value=rocket_loader_essence(value);
  auto space=[](Char c){return c==9||c==10||c==12||c==13||c==32;};
  while(!value.empty()&&space(value.front()))value.remove_prefix(1);
  while(!value.empty()&&space(value.back()))value.remove_suffix(1);
  for(std::string_view type : {"", "application/ecmascript", "application/javascript",
      "application/x-ecmascript", "application/x-javascript", "text/ecmascript",
      "text/javascript", "text/javascript1.0", "text/javascript1.1", "text/javascript1.2",
      "text/javascript1.3", "text/javascript1.4", "text/javascript1.5", "text/jscript",
      "text/livescript", "text/x-ecmascript", "text/x-javascript"}) {
    if(type.size()!=value.size())continue;
    bool same=true;
    for(size_t i=0;i<value.size();++i) {
      auto c=value[i];if(c>='A'&&c<='Z')c+=32;
      if(c!=Char(type[i])){same=false;break;}
    }
    if(same)return true;
  }
  return false;
}
}
