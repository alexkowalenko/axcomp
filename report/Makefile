TEX = xelatex -shell-escape -interaction=nonstopmode -file-line-error
TEX = xelatex --shell-escape --file-line-error

.PHONY: all view clean

SRC = report.tex comparison.tex

report.pdf :  report.tex report.sty
	$(TEX) $<
	$(TEX) $<

comparison.pdf : comparison.tex report.sty
	$(TEX) $<
	$(TEX) $<

all : report.pdf comparison.pdf

view : report.pdf comparison.pdf
	open report.pdf
	open comparison.pdf

clean :
	rm -f *.aux *.log *.toc *.idx *.ind *.out *.ilg *.pdf *.synctex.gz
	rm -f *.glo *.glsdefs *.gls *.gtn *.glg *.got *.greek
	rm -rf *.nlo *.nls
	rm -rf *.bbl *.bcf *.blg *.bak
	rm -rf *.fdb_latexmk *.fls __latexindent_temp.tex

