TEX = xelatex -shell-escape -interaction=nonstopmode -file-line-error
TEX = xelatex --shell-escape --file-line-error

.PHONY: all view clean

SRC = report.tex

report.pdf : report.sty

all : report.pdf

view : report.pdf
	open $<

clean :
	rm -f *.aux *.log *.toc *.idx *.ind *.out *.ilg *.pdf *.synctex.gz
	rm -f *.glo *.glsdefs *.gls *.gtn *.glg *.got *.greek
	rm -rf *.nlo *.nls
	rm -rf *.bbl *.bcf *.blg *.bak

report.pdf : $(SRC)
	$(TEX) $<
	$(TEX) $<


