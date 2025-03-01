#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void textureExtract(const char* texturePath) {
    struct dirent* entry;
    DIR *dir = opendir(texturePath);

    if (!dir) {
        perror("opendir failed");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        // Skip "." and ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Construct the full path
        char fullPath[1024];  // Adjust size as needed
        snprintf(fullPath, sizeof(fullPath), "%s/%s", texturePath, entry->d_name);

        printf("%s\n", fullPath);
    }

    closedir(dir);
}

int main() {
    textureExtract("src/images/idle_1");
    return 0;
}
