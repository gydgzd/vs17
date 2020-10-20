#include "stdafx.h"
#include "testRTTR.h"

int g_value = 0;

static const double pi = 3.14159;
 std::string global_text;
void set_text(const std::string& text) { global_text = text; }
const std::string& get_text() { return global_text; }