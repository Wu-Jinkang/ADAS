void writeLog(char *component, char *msg)
{
    char *filename = malloc(strlen(component) + 4 + 1);
    sprintf(filename, "%s.log", component);
    FILE *fp = getFile(LOGS_PATH, filename, "a+");
    int bytes;
    bytes = fprintf(fp, "%s\n", msg);
    if (bytes == 0)
    {
        fclose(fp);
        printf("Error when writing file.");
        exit(2);
    }
    fflush(fp);
    fclose(fp);
    free(filename);
}