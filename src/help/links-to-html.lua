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

-- A screenshot, a PNG snap.py marks 144 dpi as taken at scale 2, is shown at one pixel a screen unit, wider than the
-- text if it must be.
function Image(el)
  if not pandoc.image then
    return el
  end
  local ok, _, contents = pcall(pandoc.mediabag.fetch, el.src)
  if not ok or not contents then
    return el
  end
  local size = pandoc.image.size(contents)
  if size.dpi_horz and size.dpi_horz >= 142 and size.dpi_horz <= 146 then
    el.attributes.width = tostring(math.floor(size.width / 2 + 0.5))
    el.attributes.style = "max-width: none"
  end
  return el
end
