[1] https://github.com/Microsoft/DirectXTK12/wiki#vcpkg-c-package-manager

[2] https://devblogs.microsoft.com/cppblog/vcpkg-is-now-included-with-visual-studio/

[3] https://learn.microsoft.com/en-us/vcpkg/users/manifests

```
error: this vcpkg instance requires a manifest with a specified baseline in order to interact with ports. Please add 'builtin-baseline' to the manifest or add a 'vcpkg-configuration.json' that redefines the default registry.
```

```
vcpkg x-update-baseline --add-initial-baseline
```

stdafx is a precompiled file that should contain