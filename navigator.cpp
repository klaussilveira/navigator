#include <webkit2/webkit2.h>
#include <gtkmm.h>
#include <iostream>
#include <fstream>
#include <algorithm>

struct Config {
  std::string start;
  std::string user_agent;
};

std::string parse_address(const std::string& address)
{
  if (address.find("http://") == 0) {
    return address;
  }

  if (address.find("https://") == 0) {
    return address;
  }

  if (address.find('.') != std::string::npos) {
    return "https://" + address;
  }

  return "https://duckduckgo.com/?q=" + address;
}

void load_config(Config* config)
{
  std::string home(getenv("HOME"));
  std::ifstream file(home + "/.navrc");

  if (!file.is_open()) {
    return;
  }

  std::string line;
  while (getline(file, line)) {
    line.erase(std::remove_if(line.begin(), line.end(), isspace), line.end());
    if (line[0] == '#' || line.empty()) {
      continue;
    }

    std::string::size_type delimiterPos = line.find('=');
    std::string name = line.substr(0, delimiterPos);
    std::string value = line.substr(delimiterPos + 1);

    if (name == "start") {
      config->start = value;
    }

    if (name == "user_agent") {
      config->user_agent = value;
    }
  }
}

int main(int argc, char *argv[])
{
  Config config = {
    "https://duckduckgo.com",
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/80.0.3987.116 Safari/537.36"
  };

  Glib::RefPtr<Gtk::Application> app = Gtk::Application::create("");

  Gtk::Window window;
  window.set_default_size(1366, 768);
  load_config(&config);

  WebKitWebContext* context = webkit_web_context_new_ephemeral();
  WebKitCookieManager* cookies = webkit_web_context_get_cookie_manager(context);

  WebKitWebView* widget = WEBKIT_WEB_VIEW(webkit_web_view_new_with_context(context));
  Gtk::Widget* wrapper = Glib::wrap(GTK_WIDGET(widget));
  WebKitSettings* settings = webkit_web_view_get_settings(widget);

  // Default configuration
  webkit_web_context_set_spell_checking_enabled(context, false);
  webkit_cookie_manager_set_accept_policy(cookies, WEBKIT_COOKIE_POLICY_ACCEPT_NO_THIRD_PARTY);
  webkit_settings_set_enable_javascript(settings, false);
  webkit_settings_set_user_agent(settings, config.user_agent.c_str());

  Gtk::Box layout(Gtk::ORIENTATION_VERTICAL);
  window.add(layout);

  Gtk::Entry address;
  address.signal_activate().connect(
    [&widget, &address] () {
      webkit_web_view_load_uri(widget, parse_address(address.get_text()).c_str());
      address.hide();
    }
  );
  layout.pack_start(address, false, false);

  window.signal_key_press_event().connect(
    [&address, &widget, &settings] (GdkEventKey* event) -> bool {
      // Go back
      if (event->keyval == GDK_KEY_Escape) {
        webkit_web_view_go_back(widget);

        return true;
      }

      // Reload
      if (event->keyval == GDK_KEY_F5) {
        webkit_web_view_reload(widget);

        return true;
      }

      if ((event->state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK | GDK_MOD1_MASK)) == GDK_CONTROL_MASK) {
        // Toggle address bar visibility
        if (event->keyval == GDK_KEY_l) {
          if (address.is_visible()) {
            address.hide();
          } else {
            address.show();
            address.grab_focus();
          }

          return true;
        }

        // Toggle Javascript
        if (event->keyval == GDK_KEY_j) {
          if (webkit_settings_get_enable_javascript(settings)) {
            webkit_settings_set_enable_javascript(settings, false);
            std::cout << "Javascript disabled" << std::endl;
          } else {
            webkit_settings_set_enable_javascript(settings, true);
            std::cout << "Javascript enabled" << std::endl;
          }

          return true;
        }
      }

      return false;
    }
  , false);

  layout.pack_end(*wrapper, true, true);

  if (argc > 1) {
    webkit_web_view_load_uri(widget, parse_address(argv[1]).c_str());
  } else {
    webkit_web_view_load_uri(widget, config.start.c_str());
  }

  window.show_all();
  address.hide();
  app->run(window);
  exit(0);
}
