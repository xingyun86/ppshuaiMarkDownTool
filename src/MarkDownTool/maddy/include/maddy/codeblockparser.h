/*
 * This project is licensed under the MIT license. For more information see the
 * LICENSE file.
 */
#pragma once

// -----------------------------------------------------------------------------

#include <functional>
#include <string>
#include <regex>

#include "maddy/blockparser.h"

// -----------------------------------------------------------------------------

namespace maddy {

// -----------------------------------------------------------------------------

/**
 * CodeBlockParser
 *
 * From Markdown: 3 times surrounded code (without space in the beginning)
 *
 * ```
 *  ```
 * some code
 *  ```
 * ```
 *
 * To HTML:
 *
 * ```
 * <pre><code>
 * some code
 * </code></pre>
 * ```
 *
 * @class
 */
class CodeBlockParser : public BlockParser
{
public:
  /**
   * ctor
   *
   * @method
   * @param {std::function<void(std::string&)>} parseLineCallback
   * @param {std::function<std::shared_ptr<BlockParser>(const std::string& line)>} getBlockParserForLineCallback
   */
   CodeBlockParser(
    std::function<void(std::string&)> parseLineCallback,
    std::function<std::shared_ptr<BlockParser>(const std::string& line)> getBlockParserForLineCallback
  )
    : BlockParser(parseLineCallback, getBlockParserForLineCallback)
    , isStarted(false)
    , isFinished(false)
  {}

  /**
   * IsStartingLine
   *
   * If the line starts with three code signs, then it is a code block.
   *
   * ```
   *  ```
   * ```
   *
   * @method
   * @param {const std::string&} line
   * @return {bool}
   */
  static bool
  IsStartingLine(const std::string& line)
  {
     static std::regex re("^(?:`){3}(.*)$");
    return std::regex_match(line, re);
  }

  /**
   * IsFinished
   *
   * @method
   * @return {bool}
   */
  bool
  IsFinished() const override
  {
    return this->isFinished;
  }

protected:
  bool
  isInlineBlockAllowed() const override
  {
    return false;
  }

  bool
  isLineParserAllowed() const override
  {
    return false;
  }

  void
  parseBlock(std::string& line) override
  {
    if (line == "```")
    {
      if (!this->isStarted)
      {
        line = "<pre><code>\n";
        this->cbType = CBTYPE_NULL;
        this->isStarted = true;
        this->isFinished = false;
        return;
      }
      else
      {
          switch (this->cbType)
          {
          case CBTYPE_NULL:
          case CBTYPE_KEY:
          case CBTYPE_HTML:
          case CBTYPE_UNKNOWN:
          {
              line = "</code></pre>";
          }
          break;
          case CBTYPE_MERMAID:
          {
              line = "</div>";
          }
          break;
          case CBTYPE_PLANTUML:
          {
              line = "'>";
          }
          break;
          default:
          {
              line = "</code></pre>";
          }
          break;
          }

          this->isFinished = true;
          this->isStarted = false;
          return;
      }
    }
    else if(line == "```key")
    {
        if (!this->isStarted)
        {
            line = "<pre><code>\n";
            this->cbType = CBTYPE_KEY;
            this->isStarted = true;
            this->isFinished = false;
            return;
        }
    }
    else if(line == "```html")
    {
        if (!this->isStarted)
        {
            line = "<pre><code>\n";
            this->cbType = CBTYPE_HTML;
            this->isStarted = true;
            this->isFinished = false;
            return;
        }
    }
    else if(line == "```mermaid")
    {
        if (!this->isStarted)
        {
            line = "<div class='mermaid'>\n";
            this->cbType = CBTYPE_MERMAID;
            this->isStarted = true;
            this->isFinished = false;
            return;
        }
    }
    else if(line == "```plantuml")
    {
        if (!this->isStarted)
        {
            line = "<img uml='\n";
            this->cbType = CBTYPE_PLANTUML;
            this->isStarted = true;
            this->isFinished = false;
            return;
        }
    }
    else
    {
        if (!this->isStarted)
        {
            line = "<pre><code>\n";
            this->cbType = CBTYPE_UNKNOWN;
            this->isStarted = true;
            this->isFinished = false;
            return;
        }
    }
    line += "\n";
  }

private:
  bool isStarted;
  enum CodeBlockType
  {
      CBTYPE_NULL,
      CBTYPE_KEY,
      CBTYPE_HTML,
      CBTYPE_MERMAID,
      CBTYPE_PLANTUML,
      CBTYPE_UNKNOWN,
  } cbType;
  bool isFinished;
}; // class CodeBlockParser

// -----------------------------------------------------------------------------

} // namespace maddy
