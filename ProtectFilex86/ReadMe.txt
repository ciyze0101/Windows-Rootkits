1.Hook NtSetInformationFile to change target file
2.Hook NtWriteFile to write the target file
3.Hook NtDeleteFile to delete the target file
4.bind keyboard Filter Driver to avoid "ctrl+c" copy the content
There has a problem : the keyboard filter Driver is global , and other process cann't use "ctrl+c" also.
