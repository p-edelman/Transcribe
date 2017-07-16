function Component() {
  // default constructor
}

Component.prototype.createOperations = function() {
  // Call default constructor
  component.createOperations();

  if (systemInfo.productType === "windows") {
    component.addOperation("CreateShortcut",
                           "@TargetDir@/transcribe/Transcribe.exe",
                           "@StartMenuDir@/Transcribe.lnk",
                           "workingDirectory=@HomeDir@");
    component.addOperation("CreateShortcut",
                           "@TargetDir@/maintenancetool.exe",
                           "@StartMenuDir@/Uninstall Transcribe.lnk");
  }
}
