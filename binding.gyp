{
  "targets": [
    {
      "target_name": "autovivify",
      "sources": [
        "autovivify.cc"
      ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")"
      ]
    }
  ]
}
