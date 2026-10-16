-- links-to-html.lua
function Link(el)
  el.target = string.gsub(el.target, "%.md", ".html")
  return el
end

-- Every page but the contents opens with a link back to it.
function Pandoc(doc)
  local input = PANDOC_STATE.input_files[1] or ""
  if not string.match(input, "index%.md$") then
    local back = pandoc.Para({ pandoc.Link("Contents", "index.html") })
    table.insert(doc.blocks, 1, back)
  end
  return doc
end
