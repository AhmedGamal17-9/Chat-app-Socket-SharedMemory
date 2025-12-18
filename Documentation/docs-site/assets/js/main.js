document.addEventListener("DOMContentLoaded", () => {
  // Theme Toggle
  const themeToggle = document.getElementById("theme-toggle");
  const html = document.documentElement;
  const storedTheme = localStorage.getItem("theme");

  if (
    storedTheme === "dark" ||
    (!storedTheme && window.matchMedia("(prefers-color-scheme: dark)").matches)
  ) {
    html.classList.add("dark");
  }

  themeToggle.addEventListener("click", () => {
    html.classList.toggle("dark");
    localStorage.setItem(
      "theme",
      html.classList.contains("dark") ? "dark" : "light"
    );
  });

  // Mobile Menu
  const menuBtn = document.getElementById("menu-btn");
  const sidebar = document.getElementById("sidebar");
  const overlay = document.getElementById("mobile-overlay");

  function toggleMenu() {
    const isClosed = sidebar.classList.contains("-translate-x-full");
    if (isClosed) {
      sidebar.classList.remove("-translate-x-full");
      overlay.classList.remove("hidden");
    } else {
      sidebar.classList.add("-translate-x-full");
      overlay.classList.add("hidden");
    }
  }

  menuBtn.addEventListener("click", toggleMenu);
  overlay.addEventListener("click", toggleMenu);

  sidebar.querySelectorAll("a").forEach((link) => {
    link.addEventListener("click", () => {
      if (window.innerWidth < 1024) {
        toggleMenu();
      }
    });
  });

  // Copy to Clipboard
  document.querySelectorAll("pre").forEach((pre) => {
    const button = document.createElement("button");
    button.className =
      "absolute top-2 right-2 p-1 rounded-md bg-gray-200 dark:bg-slate-700 text-gray-500 dark:text-gray-400 opacity-0 group-hover:opacity-100 transition-opacity";
    button.innerHTML =
      '<svg class="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24"><path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M8 16H6a2 2 0 01-2-2V6a2 2 0 012-2h8a2 2 0 012 2v2m-6 12h8a2 2 0 002-2v-8a2 2 0 00-2-2h-8a2 2 0 00-2 2v8a2 2 0 002 2z"></path></svg>';

    button.addEventListener("click", () => {
      const code = pre.querySelector("code").innerText;
      navigator.clipboard.writeText(code);
      button.classList.add("text-green-500");
      setTimeout(() => button.classList.remove("text-green-500"), 2000);
    });

    const wrapper = pre.parentElement;
    if (wrapper.classList.contains("group")) {
      wrapper.appendChild(button);
    }
  });

  // Highlighting Active Search Result
  const sections = document.querySelectorAll("section");
  const navLinks = document.querySelectorAll("#nav-links a");

  const observer = new IntersectionObserver(
    (entries) => {
      entries.forEach((entry) => {
        if (entry.isIntersecting) {
          const id = entry.target.id;
          navLinks.forEach((link) => {
            link.classList.remove(
              "bg-gray-100",
              "dark:bg-slate-800",
              "text-blue-600",
              "dark:text-blue-400"
            );
            if (link.getAttribute("href") === `#${id}`) {
              link.classList.add(
                "bg-gray-100",
                "dark:bg-slate-800",
                "text-blue-600",
                "dark:text-blue-400"
              );
            }
          });
        }
      });
    },
    { threshold: 0.1, rootMargin: "-20% 0px -50% 0px" }
  );

  sections.forEach((section) => observer.observe(section));

  // Search Functionality with Highlighting
  const searchInput = document.getElementById("search-input");
  const searchResults = document.getElementById("search-results");
  let searchIndex = [];

  sections.forEach((section) => {
    const id = section.id;
    const title = section.querySelector("h2")?.innerText || id;
    const content = section.innerText;
    searchIndex.push({ id, title, content });
  });

  searchInput.addEventListener("input", (e) => {
    const query = e.target.value.toLowerCase();
    
    // Clear previous highlights
    clearHighlights();

    if (query.length < 2) {
      searchResults.classList.add("hidden");
      return;
    }

    const results = searchIndex
      .filter(
        (item) =>
          item.title.toLowerCase().includes(query) ||
          item.content.toLowerCase().includes(query)
      )
      .slice(0, 5);

    searchResults.innerHTML = results
      .map(
        (item) => `
            <a href="#${item.id}" class="block px-4 py-2 hover:bg-gray-100 dark:hover:bg-slate-700 search-result-item" data-query="${query}">
                <div class="text-sm font-bold text-gray-900 dark:text-white">${item.title}</div>
                <div class="text-xs text-gray-500 truncate">${item.content.substring(0, 60)}...</div>
            </a>
        `
      )
      .join("");

    if (results.length > 0) {
      searchResults.classList.remove("hidden");
    } else {
      searchResults.classList.add("hidden");
    }
  });

  // Handle Search Result Click for Highlighting
  searchResults.addEventListener("click", (e) => {
    const link = e.target.closest("a");
    if (link) {
      const query = searchInput.value;
      if (query) {
        highlightTerm(query);
      }
      searchResults.classList.add("hidden");
    }
  });

  function highlightTerm(term) {
    if (!term) return;
    
    // Use TreeWalker to find text nodes
    const walker = document.createTreeWalker(
        document.querySelector('.prose'), 
        NodeFilter.SHOW_TEXT, 
        null, 
        false
    );

    const nodesToReplace = [];
    let node;
    while(node = walker.nextNode()) {
        if (node.nodeValue.toLowerCase().includes(term.toLowerCase()) && node.parentElement.tagName !== 'SCRIPT' && node.parentElement.tagName !== 'STYLE') {
            nodesToReplace.push(node);
        }
    }

    nodesToReplace.forEach(node => {
        const span = document.createElement('span');
        const regex = new RegExp(`(${term})`, 'gi');
        span.innerHTML = node.nodeValue.replace(regex, '<span class="search-highlight">$1</span>');
        node.parentElement.replaceChild(span, node);
    });
  }

  function clearHighlights() {
    const highlights = document.querySelectorAll('.search-highlight');
    highlights.forEach(mark => {
        const parent = mark.parentElement;
        parent.replaceChild(document.createTextNode(mark.textContent), mark);
        // Normalize to merge text nodes
        parent.parentElement.normalize(); 
    });
  }

  document.addEventListener("keydown", (e) => {
    if ((e.ctrlKey || e.metaKey) && e.key === "k") {
      e.preventDefault();
      searchInput.focus();
    }
  });

  document.addEventListener("click", (e) => {
    if (!searchInput.contains(e.target) && !searchResults.contains(e.target)) {
      searchResults.classList.add("hidden");
    }
  });

  // Deep Linking for Headers
  document.querySelectorAll(".prose h2, .prose h3").forEach((header) => {
     // Check if header already has an ID, if not, generate one? 
     // (Most sections have IDs, headers usually don't but are inside sections which is fine. 
     //  We added ID to sections, so we can link to section. 
     //  But requirement says "link icon next to all section headings".)
     
     // Note: In index.html, headers are inside <section id="...">. 
     // We can just link to that parent ID.
     const section = header.closest("section");
     if (section && section.id) {
         const anchor = document.createElement("a");
         anchor.href = `#${section.id}`;
         anchor.className = "anchor-link";
         anchor.innerHTML = "#";
         anchor.title = "Copy link to this section";
         anchor.addEventListener('click', (e) => {
             e.preventDefault();
             navigator.clipboard.writeText(window.location.origin + window.location.pathname + anchor.href);
             
             // Visual feedback
             const originalText = anchor.innerHTML;
             anchor.innerHTML = "Copied!";
             setTimeout(() => anchor.innerHTML = originalText, 1500);
             
             // Actually navigate
             window.location.hash = section.id;
         });
         header.prepend(anchor);
     }
  });

  // Tabbed Interface
  const tabs = document.querySelectorAll('[role="tab"]');
  tabs.forEach(tab => {
    tab.addEventListener('click', () => {
        const targetSelector = tab.getAttribute('data-tabs-target');
        const targetContent = document.querySelector(targetSelector);
        
        // Hide all contents in this group
        const parentList = tab.closest('ul');
        const grandParent = parentList.parentElement.parentElement; // section
        const contentContainer = grandParent.querySelector('#build-tab-content'); // Rigid selector for this specific usage
        
        // Deactivate all tabs
        parentList.querySelectorAll('[role="tab"]').forEach(t => {
            t.classList.remove('text-blue-600', 'border-blue-600', 'dark:text-blue-500', 'dark:border-blue-500', 'active', 'bg-white', 'dark:bg-slate-800');
            t.classList.add('border-transparent', 'hover:text-gray-600', 'hover:border-gray-300', 'dark:hover:text-gray-300');
            t.setAttribute('aria-selected', 'false');
        });
        
        // Hide all panels
        contentContainer.querySelectorAll('[role="tabpanel"]').forEach(p => p.classList.add('hidden'));

        // Activate clicked tab
        tab.classList.remove('border-transparent', 'hover:text-gray-600', 'hover:border-gray-300', 'dark:hover:text-gray-300');
        tab.classList.add('text-blue-600', 'border-blue-600', 'dark:text-blue-500', 'dark:border-blue-500', 'active', 'bg-white', 'dark:bg-slate-800');
        tab.setAttribute('aria-selected', 'true');
        
        // Show target panel
        targetContent.classList.remove('hidden');
    });
  });

  // Back to Top Button
  const backToTopBtn = document.getElementById("back-to-top");
  const mainContent = document.querySelector("main"); // The scrollable container
  
  if (backToTopBtn && mainContent) {
      mainContent.addEventListener("scroll", () => {
        if (mainContent.scrollTop > 300) {
            backToTopBtn.classList.add("visible");
        } else {
            backToTopBtn.classList.remove("visible");
        }
      });

      backToTopBtn.addEventListener("click", () => {
        mainContent.scrollTo({
            top: 0,
            behavior: "smooth"
        });
      });
  }

  // --- NEW FEATURES ---

  // Font Size Controls
  const fontDecBtn = document.getElementById('font-dec');
  const fontResetBtn = document.getElementById('font-reset');
  const fontIncBtn = document.getElementById('font-inc');
  const root = document.documentElement;

  // Load saved font size
  const savedFontSize = localStorage.getItem('fontSize');
  if (savedFontSize) {
      root.style.fontSize = savedFontSize;
  }

  if (fontDecBtn && fontResetBtn && fontIncBtn) {
      fontDecBtn.addEventListener('click', () => {
          let current = parseFloat(getComputedStyle(root).fontSize);
          let newSize = Math.max(12, current - 2); // Min 12px
          root.style.fontSize = newSize + 'px';
          localStorage.setItem('fontSize', newSize + 'px');
      });

      fontResetBtn.addEventListener('click', () => {
          root.style.fontSize = ''; // Reset to stylesheet default (usually 16px/100%)
          localStorage.removeItem('fontSize');
      });

      fontIncBtn.addEventListener('click', () => {
          let current = parseFloat(getComputedStyle(root).fontSize);
          let newSize = Math.min(24, current + 2); // Max 24px
          root.style.fontSize = newSize + 'px';
          localStorage.setItem('fontSize', newSize + 'px');
      });
  }

  // Buffer Calculator
  const calcTriggerBtn = document.getElementById('calc-trigger-btn');
  const calcModal = document.getElementById('calc-modal');
  const calcCloseBtn = document.getElementById('calc-close-btn');
  const calcTextInput = document.getElementById('calc-text-input');
  const calcType = document.getElementById('calc-type');
  const calcResult = document.getElementById('calc-result');

  function updateCalculation() {
      const text = calcTextInput.value || "";
      const numberBytes = parseInt(calcType.value) || 1;
      
      let totalBytes = 0;
      
      for (let i = 0; i < text.length; i++) {
          const char = text[i];
          
          if (/[\u0600-\u06FF]/.test(char)) {
              // Arabic Character -> 2 bytes
              totalBytes += 2;
          } else if (/[0-9]/.test(char)) {
              // Number -> Depends on selection
              totalBytes += numberBytes;
          } else {
              // English / Other -> 1 byte
              totalBytes += 1;
          }
      }
      
      calcResult.innerText = totalBytes + " bytes";
  }

  if (calcTriggerBtn && calcModal && calcCloseBtn) {
      calcTriggerBtn.addEventListener('click', () => {
          calcModal.classList.remove('hidden');
      });

      calcCloseBtn.addEventListener('click', () => {
          calcModal.classList.add('hidden');
      });
      
      // Close on background click
      calcModal.addEventListener('click', (e) => {
        if (e.target === calcModal.querySelector('.bg-gray-500.bg-opacity-75')) {
             calcModal.classList.add('hidden');
        }
        // Also the wrapper div
        if (e.target.closest('.fixed.inset-0.z-10') && !e.target.closest('.rounded-lg')) {
            calcModal.classList.add('hidden');
        }
      });

      calcTextInput.addEventListener('input', updateCalculation);
      calcType.addEventListener('change', updateCalculation);
  }

});
