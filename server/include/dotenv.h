#pragma once

// Load environment variables from a .env file for development.
// This is a minimal loader supporting lines like KEY=VALUE and ignoring comments (#...).
// It looks for, in order: "./.env" and "./server/.env" relative to current working directory.
// It is safe to call multiple times; variables in the file will overwrite existing values.
void load_dotenv_if_present();
